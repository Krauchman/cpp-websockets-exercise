#ifdef STAND_ALONE
    #define BOOST_TEST_MODULE TickerLoggerTests
#endif
#include <boost/test/unit_test.hpp>

#include "mock_session.h"

#include <coinbase/subscribe.h>
#include <websocket/websocket.h>

BOOST_AUTO_TEST_SUITE(test_suite_subscriber)

    BOOST_AUTO_TEST_CASE(test_case_subscribe)
    {
        auto session = std::make_unique<mock_session>(std::vector<std::string>{
            R"({"type": "subscriptions"})",
            R"({"type": "ticker", "side": "sell"})",
            R"({"type": "ticker", "product_id": "ETH-USD"})"
        });
        EXPECT_CALL(*session, start()).Times(1);
        EXPECT_CALL(*session, write(R"({"channels":["ticker"],"product_ids":["ETH-USD","TEST-EUR"],"type":"subscribe"})")).Times(1);

        coinbase::subscribe::ticker_subscriber subscriber(std::move(session), std::vector<std::string>{"ETH-USD", "TEST-EUR"});

        std::vector<std::string> expected_messages{
            R"({"type": "ticker", "side": "sell"})",
            R"({"type": "ticker", "product_id": "ETH-USD"})"
        };

        std::vector<std::string> actual_messages;
        try {
            subscriber.start([&actual_messages](const std::string& message) {
                actual_messages.push_back(message);
            });
        }
        catch (out_of_messages_exception e) {}

        BOOST_CHECK_EQUAL_COLLECTIONS(actual_messages.begin(), actual_messages.end(), expected_messages.begin(), expected_messages.end());
    }

BOOST_AUTO_TEST_SUITE_END()
