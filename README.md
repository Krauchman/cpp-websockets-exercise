C++ websockets exercise
=



Overview
-
Subscribes to a Coinbase ticker channel and logs all of the incoming messages in a separate thread.



Build
-
1. C++14 and Boost >= 1.74 are required in order to run the app. Boost installation command for Unix variants:
    ```
    sudo apt-get install libboost-all-dev
    ```
    Other two dependencies, `GMock` and `nlohmann/json`, are auto-fetched on build.

1. Build with CMake:
    ```
    mkdir build
    cd build
    cmake ..
    make
    ```

1. The app binary will be located at `<repo_root>/build/ticker_logger`

1. Unit-tests binary will be located at `<repo_root>/build/test/test_ticker_logger`



Run
-
- For available options:
    ```
    $ ./ticker_logger --help

    Allowed options:
        --help                  produce help message
        -o [ --output ] arg     output file
        -p [ --product_id ] arg product IDs (multiple)
        --use_lock_queue        Use regular queue with synchronization primitives 
                                instead of lock-free one
    ```

- Example 1 - subscribe to two products and log to `output.csv`:
    ```
    ./ticker_logger -p BTC-USD -p ETH-USD -o output.csv
    ```

- Example 2 - use another thread-safe queue implementation:
    ```
    ./ticker_logger -p BTC-USD -o output.csv --use_lock_queue
    ```