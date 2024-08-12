#ifdef STAND_ALONE
    #define BOOST_TEST_MODULE TickerLoggerTests
#endif
#include <boost/test/unit_test.hpp>

#include <coinbase/message.h>

BOOST_AUTO_TEST_SUITE(test_suite_coinbase_message)
    using namespace coinbase::message;

    BOOST_AUTO_TEST_CASE(test_case_csv_functions) {
        ticker_message message(R"({
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
        })");

        std::string expected_row_str = R"("ticker",37475248783,"ETH-USD","1285.22",)"
                R"("1310.79","245532.79269678","1280.52","1313.8",)"
                R"("9788783.60117027","1285.04","0.46688654","1285.27",)"
                R"("1.56637040","buy","2022-10-19T23:28:22.061769Z",370843401,)"
                R"("11.4396987")";

        BOOST_CHECK_EQUAL(message.to_csv_row(), expected_row_str);
    }

BOOST_AUTO_TEST_SUITE_END()
