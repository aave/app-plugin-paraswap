#pragma once

#include <string.h>
#include "eth_internals.h"
#include "eth_plugin_interface.h"

// TODO: unknown
#define PARAMETER_LENGTH 32

// TODO: unknown
#define RUN_APPLICATION 1

#define NUM_AAVE_SELECTORS 6
#define SELECTOR_SIZE          4

#define PLUGIN_NAME "Aave"

// TODO: unknown
#define TOKEN_SENT_FOUND     1
// TODO: unknown
#define TOKEN_RECEIVED_FOUND 1 << 1

// We use `0xeeeee` as a dummy address to represent ETH.
extern const uint8_t MOCK_ETH_ADDRESS[ADDRESS_LENGTH];

// Address 0x00000... used to indicate that the beneficiary is the sender.
extern const uint8_t NULL_ETH_ADDRESS[ADDRESS_LENGTH];

// Used to indicate that user will interact with the variable borrow rate mode
extern const uint8_t VARIABLE_BORROW_RATE_MODE_RAW[PARAMETER_LENGTH];

// Used to indicate that user will interact with the stable borrow rate mode
extern const uint8_t STABLE_BORROW_RATE_MODE_RAW[PARAMETER_LENGTH];

// Returns 1 if corresponding address is the mock address for ETH (0xeeeee...).
#define ADDRESS_IS_ETH(_addr) (!memcmp(_addr, MOCK_ETH_ADDRESS, ADDRESS_LENGTH))

// Returns 1 if corresponding address is empty (0x000000...).
#define ADDRESS_IS_NULL(_addr) (!memcmp(_addr, NULL_ETH_ADDRESS, ADDRESS_LENGTH))

// Returns 1 if corresponding borrow rate mode are variable.
#define BORROW_RATE_MODE_IS_VARIABLE(_mode) (!memcmp(_mode, VARIABLE_BORROW_RATE_MODE_RAW, PARAMETER_LENGTH))

// Returns 1 if corresponding borrow rate mode are stable.
#define BORROW_RATE_MODE_IS_STABLE(_mode) (!memcmp(_mode, STABLE_BORROW_RATE_MODE_RAW, PARAMETER_LENGTH))

typedef enum {
    DEPOSIT,
    BORROW,
    REPAY,
    WITHDRAW,
    SWAP_BORROW_RATE,
    SET_USAGE_AS_COLLATERAL,
} aaveSelector_t;
typedef enum {
    RATE_MODE_NOT_SET,
    VARIABLE,
    STABLE,
} interestRateMode_t;

typedef enum {
    SEND_SCREEN,
    RECEIVE_SCREEN,
    WARN_SCREEN,
    BENEFICIARY_SCREEN,
    ERROR,
} screens_t;

//PARAMS OF THE SIMPLE METHODS
// Would've loved to make this an enum but we don't have enough room because enums are `int` and not
// `uint8_t`.
#define ASSET     0  // Address of the token the user is sending.
#define AMOUNT 1  // Amount sent by the contract to the user.
#define ON_BEHALF_OF_DEPOSIT      2  // Beneficiary of the deposit.
#define REFERRAL_CODE      3  // Referral code of the integrator

#define INTEREST_RATE_MODE      2  // Address of the token the user is sending.
#define ON_BEHALF_OF_BORROW      4  // Address of the token the user is sending.

#define ON_BEHALF_OF_REPAY      3  // Address of the token the user is sending.

#define INTEREST_RATE_MODE_SWAP     1  // The rate mode that the user wants to swap to

#define USAGE_AS_COLLATERAL_MODE     1  // The state that the user wants to swap to

#define TOKEN_RECEIVED  3  // Address of the token sent to the user.
#define PATH \
    4  // Path of the different asseths that will get swapped during the trade. First and last
       // tokens are the ones we care about.
#define BENEFICIARY           5  // Address to which the contract will send the tokens.
#define OFFSET                6
#define PATHS_OFFSET          7
#define PATHS_LEN             8
#define MEGA_PATHS_OFFSET     9
#define MEGA_PATHS_LEN        10
#define FIRST_MEGAPATH_OFFSET 11
#define FIRST_MEGAPATH        12
#define NONE                  13  // Placeholder variant to be set when parsing is done but data is still being sent.

// Number of decimals used when the token wasn't found in the CAL.
#define DEFAULT_DECIMAL WEI_TO_ETHER

// Ticker used when the token wasn't found in the CAL.
#define DEFAULT_TICKER ""

// Shared global memory with Ethereum app. Must be at most 5 * 32 bytes.
typedef struct aave_parameters_t {
    uint8_t amount_sent[INT256_LENGTH];
    uint8_t amount_received[INT256_LENGTH];
    char beneficiary[ADDRESS_LENGTH];
    uint8_t contract_address_sent[ADDRESS_LENGTH];
    uint8_t contract_address_received[ADDRESS_LENGTH];
    char ticker_sent[MAX_TICKER_LEN];
    char ticker_received[MAX_TICKER_LEN];
    uint8_t interest_rate_mode;
    // 32 * 2 + 20 * 3 + 12 * 2 == 64 + 60 + 24 == 144
    // 32 * 5 == 160 bytes so there are 160 - 144 == 16 bytes left.

    uint8_t next_param;
    uint8_t tokens_found;
    uint8_t valid;
    uint8_t decimals_sent;
    uint8_t decimals_received;
    uint8_t selectorIndex;
    // 4 * 1 + 2 * 2 + 7 * 1 == 8 + 7 == 15 bytes. There are 16 - 15 == 1 byte left.
} aave_parameters_t;

void handle_provide_parameter(void *parameters);
void handle_query_contract_ui(void *parameters);
void paraswap_plugin_call(int message, void *parameters);