#include "support/ServerProcess.hpp"
#include "support/TestClient.hpp"

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <memory>
#include <sstream>
#include <thread>

using namespace test;
namespace fs = std::filesystem;

class ServerBehaviorTest : public ::testing::Test {
protected:
    void SetUp() override {
        ASSERT_TRUE(serverPortIsFree())
            << "port " << kServerPort << " is already in use, stop any running server first";

        auto testName = ::testing::UnitTest::GetInstance()->current_test_info()->name();
        workDir_ = fs::temp_directory_path() / (std::string("tcp-chat-test-") + testName);
        fs::remove_all(workDir_);
        fs::create_directories(workDir_);

        server_ = std::make_unique<ServerProcess>(SERVER_EXE, workDir_);

        auto deadline = Clock::now() + 5s;
        while (serverPortIsFree()) {
            ASSERT_TRUE(server_->isRunning()) << "server exited during startup";
            ASSERT_LT(Clock::now(), deadline) << "server did not start listening";
            std::this_thread::sleep_for(10ms);
        }
    }

    void TearDown() override {
        server_.reset();
        std::error_code ignored;
        fs::remove_all(workDir_, ignored);
    }

    std::unique_ptr<TestClient> connectAs(const std::string& username) {
        auto client = std::make_unique<TestClient>();
        EXPECT_TRUE(client->login(username));
        return client;
    }

    // The server writes the log after sending, so a client can see a message before it's logged
    ::testing::AssertionResult waitForLogLine(const std::string& expected) const {
        std::string content;
        auto deadline = Clock::now() + 2s;
        while (Clock::now() < deadline) {
            std::ifstream file(workDir_ / "chat.log");
            std::stringstream buffer;
            buffer << file.rdbuf();
            content = buffer.str();
            if (content.find(expected) != std::string::npos) return ::testing::AssertionSuccess();
            std::this_thread::sleep_for(20ms);
        }
        return ::testing::AssertionFailure()
               << "chat.log never contained \"" << expected << "\", content:\n" << content;
    }

    fs::path workDir_;
    std::unique_ptr<ServerProcess> server_;
};

// ── Connection and username ──

TEST_F(ServerBehaviorTest, PromptsForUsernameOnConnect) {
    TestClient client;
    EXPECT_TRUE(client.waitFor("Enter your username: "));
}

TEST_F(ServerBehaviorTest, WelcomesUserAfterValidUsername) {
    TestClient client;
    ASSERT_TRUE(client.waitFor("Enter your username: "));
    client.send("Alice");
    EXPECT_TRUE(client.waitFor("Welcome Alice! You are now connected.\n"));
}

TEST_F(ServerBehaviorTest, TrimsTrailingNewlineFromUsername) {
    TestClient client;
    ASSERT_TRUE(client.waitFor("Enter your username: "));
    client.send("Alice\r\n");
    EXPECT_TRUE(client.waitFor("Welcome Alice! You are now connected.\n"));
}

TEST_F(ServerBehaviorTest, RejectsUsernameStartingWithSlash) {
    TestClient client;
    ASSERT_TRUE(client.waitFor("Enter your username: "));
    client.send("/admin");
    EXPECT_TRUE(client.waitFor("Error: invalid username"));
    EXPECT_TRUE(client.waitForDisconnect());
}

TEST_F(ServerBehaviorTest, RejectsEmptyUsername) {
    TestClient client;
    ASSERT_TRUE(client.waitFor("Enter your username: "));
    client.send("\r\n");
    EXPECT_TRUE(client.waitFor("Error: invalid username"));
    EXPECT_TRUE(client.waitForDisconnect());
}

TEST_F(ServerBehaviorTest, AnnouncesJoinToOtherClients) {
    auto alice = connectAs("Alice");
    auto bob = connectAs("Bob");
    EXPECT_TRUE(alice->waitFor("*** Bob has joined the chat ***"));
}

// ── Public messages ──

TEST_F(ServerBehaviorTest, BroadcastsMessageToOtherClients) {
    auto alice = connectAs("Alice");
    auto bob = connectAs("Bob");
    auto carol = connectAs("Carol");

    alice->send("Hello everyone");

    EXPECT_TRUE(bob->waitFor("[Alice]: Hello everyone"));
    EXPECT_TRUE(carol->waitFor("[Alice]: Hello everyone"));
}

TEST_F(ServerBehaviorTest, DoesNotEchoMessageToSender) {
    auto alice = connectAs("Alice");
    auto bob = connectAs("Bob");

    alice->send("Hello");

    ASSERT_TRUE(bob->waitFor("[Alice]: Hello"));
    EXPECT_FALSE(alice->waitFor("[Alice]: Hello", 300ms));
}

// ── Commands ──

TEST_F(ServerBehaviorTest, UsersCommandListsConnectedUsers) {
    auto alice = connectAs("Alice");
    auto bob = connectAs("Bob");

    alice->send("/users");

    EXPECT_TRUE(alice->waitFor("Connected users (2):\n"));
    EXPECT_TRUE(alice->waitFor("  - Alice\n"));
    EXPECT_TRUE(alice->waitFor("  - Bob\n"));
}

TEST_F(ServerBehaviorTest, HelpCommandListsCommands) {
    auto alice = connectAs("Alice");

    alice->send("/help");

    EXPECT_TRUE(alice->waitFor("Available commands:\n"));
    EXPECT_TRUE(alice->waitFor("/users"));
    EXPECT_TRUE(alice->waitFor("/msg <user> <msg>"));
    EXPECT_TRUE(alice->waitFor("/help"));
    EXPECT_TRUE(alice->waitFor("/quit"));
}

TEST_F(ServerBehaviorTest, CommandsAreNotBroadcast) {
    auto alice = connectAs("Alice");
    auto bob = connectAs("Bob");

    alice->send("/help");

    ASSERT_TRUE(alice->waitFor("Available commands:"));
    EXPECT_FALSE(bob->waitFor("/help", 300ms));
}

// ── Private messages ──

TEST_F(ServerBehaviorTest, PrivateMessageReachesOnlyTarget) {
    auto alice = connectAs("Alice");
    auto bob = connectAs("Bob");
    auto carol = connectAs("Carol");

    alice->send("/msg Bob secret plan");

    EXPECT_TRUE(bob->waitFor("[PM from Alice]: secret plan"));
    EXPECT_TRUE(alice->waitFor("[PM to Bob]: secret plan"));
    EXPECT_FALSE(carol->waitFor("secret plan", 300ms));
}

TEST_F(ServerBehaviorTest, PrivateMessageToUnknownUserReturnsError) {
    auto alice = connectAs("Alice");

    alice->send("/msg Zed hello");

    EXPECT_TRUE(alice->waitFor("Error: user \"Zed\" not found\n"));
}

TEST_F(ServerBehaviorTest, PrivateMessageWithoutArgumentsShowsUsage) {
    auto alice = connectAs("Alice");

    alice->send("/msg");

    EXPECT_TRUE(alice->waitFor("Usage: /msg <username> <message>\n"));
}

TEST_F(ServerBehaviorTest, PrivateMessageWithoutTextShowsUsage) {
    auto alice = connectAs("Alice");
    auto bob = connectAs("Bob");

    alice->send("/msg Bob");

    EXPECT_TRUE(alice->waitFor("Usage: /msg <username> <message>\n"));
}

// ── Leaving ──

TEST_F(ServerBehaviorTest, QuitSaysGoodbyeAndClosesConnection) {
    auto alice = connectAs("Alice");

    alice->send("/quit");

    EXPECT_TRUE(alice->waitFor("Goodbye Alice!\n"));
    EXPECT_TRUE(alice->waitForDisconnect());
}

TEST_F(ServerBehaviorTest, QuitAnnouncesLeaveToOthers) {
    auto alice = connectAs("Alice");
    auto bob = connectAs("Bob");

    alice->send("/quit");

    EXPECT_TRUE(bob->waitFor("*** Alice has left the chat ***"));
}

TEST_F(ServerBehaviorTest, AbruptDisconnectAnnouncesLeaveToOthers) {
    auto alice = connectAs("Alice");
    auto bob = connectAs("Bob");

    bob.reset();

    EXPECT_TRUE(alice->waitFor("*** Bob has left the chat ***"));
}

TEST_F(ServerBehaviorTest, DisconnectedUserIsRemovedFromUserList) {
    auto alice = connectAs("Alice");
    auto bob = connectAs("Bob");

    bob.reset();
    ASSERT_TRUE(alice->waitFor("*** Bob has left the chat ***"));

    // The leave notice is sent just before the server removes the client, so allow a short delay
    bool removed = false;
    auto deadline = Clock::now() + 2s;
    while (!removed && Clock::now() < deadline) {
        alice->send("/users");
        removed = alice->waitFor("Connected users (1):", 200ms);
    }
    EXPECT_TRUE(removed);
}

// ── Chat log ──

TEST_F(ServerBehaviorTest, LogsActivityToChatLog) {
    auto alice = connectAs("Alice");
    auto bob = connectAs("Bob");

    alice->send("Hello");
    ASSERT_TRUE(bob->waitFor("[Alice]: Hello"));
    alice->send("/msg Bob psst");
    ASSERT_TRUE(bob->waitFor("[PM from Alice]: psst"));

    EXPECT_TRUE(waitForLogLine("Alice has joined the chat"));
    EXPECT_TRUE(waitForLogLine("Bob has joined the chat"));
    EXPECT_TRUE(waitForLogLine("[Alice]: Hello"));
    EXPECT_TRUE(waitForLogLine("[PM] Alice -> Bob: psst"));
}
