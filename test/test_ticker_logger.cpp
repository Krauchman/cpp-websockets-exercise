#ifdef STAND_ALONE
    #define BOOST_TEST_MODULE TickerLoggerTests
#endif
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

#include "mock_session.h"

#include <coinbase/subscribe.h>
#include <coinbase/ticker_logger.h>
#include <websocket/websocket.h>

#include <fstream>
#include <sstream>
#include <iostream>

BOOST_AUTO_TEST_SUITE(test_suite_full_flow)

    namespace bdata = boost::unit_test::data;

    BOOST_AUTO_TEST_CASE(test_case_csv_file) {
        auto session = std::make_unique<mock_session>(std::vector<std::string>{
            R"({"type": "subscriptions"})",
            R"({"type": "ticker", "side": "sell"})",
            R"({"type": "ticker", "product_id": "ETH-USD"})",
            R"({
                "type": "ticker",
                "sequence": 37475248783,
                "product_id": "ETH-USD",
                "price": "1285.22",
                "open_24h": "1310.79",
                "volume_24h": "245532.79269678",
                "low_24h": "1280.52",
                "high_24h": "1313.8",
                "volume_30d": "9788783.60117027",
                "best_bid": "1285.04",
                "best_bid_size": "0.46688654",
                "best_ask": "1285.27",
                "best_ask_size": "1.56637040",
                "side": "buy",
                "time": "2022-10-19T23:28:22.061769Z",
                "trade_id": 370843401,
                "last_size": "11.4396987"
            })"
        });
        EXPECT_CALL(*session, start()).Times(1);
        EXPECT_CALL(*session, write(R"({"channels":["ticker"],"product_ids":["TEST-USD","TEST-EUR"],"type":"subscribe"})")).Times(1);

        std::string output_file_name = "test_runner.csv";
        coinbase::ticker_logger::config conf{output_file_name, {"TEST-USD", "TEST-EUR"}, false};

        coinbase::ticker_logger::runner runner(std::move(session), conf);

        try {
            runner.start();
        }
        catch (out_of_messages_exception e) {}

        std::ifstream ifs(output_file_name);
        std::string file_content((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));

        std::string expected_content = R"()"
R"(type,sequence,product_id,price,open_24h,)"
R"(volume_24h,low_24h,high_24h,volume_30d,)"
R"(best_bid,best_bid_size,best_ask,best_ask_size,)"
R"(side,time,trade_id,last_size
"ticker",,,,,,,,,,,,,"sell",,,
"ticker",,"ETH-USD",,,,,,,,,,,,,,
"ticker",37475248783,"ETH-USD","1285.22",)"
R"("1310.79","245532.79269678","1280.52",)"
R"("1313.8","9788783.60117027","1285.04",)"
R"("0.46688654","1285.27","1.56637040","buy",)"
R"("2022-10-19T23:28:22.061769Z",370843401,"11.4396987"
)";

        BOOST_CHECK_EQUAL(file_content, expected_content);
    }

    BOOST_DATA_TEST_CASE(test_case_correct_concurrency, bdata::xrange(2), use_lock_queue) {
        size_t messages_count = 300;

        std::vector<std::string> messages;
        messages.push_back(R"({"type": "subscriptions"})");
        for (size_t i = 0; i < messages_count; ++i) {
            std::stringstream ss;
            ss << R"({"type": "ticker", "sequence": )" << i << "}";
            messages.push_back(ss.str());
        }

        auto session = std::make_unique<mock_session>(messages);
        EXPECT_CALL(*session, start()).Times(1);
        EXPECT_CALL(*session, write(R"({"channels":["ticker"],"product_ids":["BTC-USD"],"type":"subscribe"})")).Times(1);

        std::string output_file_name = "test_runner.csv";
        coinbase::ticker_logger::config conf{output_file_name, {"BTC-USD"}, bool(use_lock_queue)};

        coinbase::ticker_logger::runner runner(std::move(session), conf);

        try {
            runner.start();
        }
        catch (out_of_messages_exception e) {}

        std::ifstream ifs(output_file_name);

        std::string line;
        std::getline(ifs, line); // header

        size_t line_count = 0;
        while (std::getline(ifs, line)) {
            std::stringstream expected_ss;
            expected_ss << R"("ticker",)" << line_count << R"(,,,,,,,,,,,,,,,)";

            BOOST_CHECK_EQUAL(line, expected_ss.str());
            
            ++line_count;
        }

        BOOST_CHECK_EQUAL(line_count, messages_count);
    }

BOOST_AUTO_TEST_SUITE_END()
