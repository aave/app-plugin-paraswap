#include "paraswap_plugin.h"
#include <string.h>

// Need more information about the interface for plugins? Please read the README.md!

static const uint8_t AAVE_DEPOSIT_SELECTOR[SELECTOR_SIZE] = {0xe8, 0xed, 0xa9, 0xdf};
static const uint8_t AAVE_BORROW_SELECTOR[SELECTOR_SIZE] = {0xa4, 0x15, 0xbc, 0xad};
static const uint8_t AAVE_REPAY_SELECTOR[SELECTOR_SIZE] = {0x57, 0x3a, 0xde, 0x81};
static const uint8_t AAVE_WITHDRAW_SELECTOR[SELECTOR_SIZE] = {0x69, 0x32, 0x8d, 0xec};

static const uint8_t AAVE_SWAP_BORROW_RATE_SELECTOR[SELECTOR_SIZE] = {0x94, 0xba, 0x89, 0xa2};
static const uint8_t AAVE_SET_USAGE_AS_COLLATERAL_SELECTOR[SELECTOR_SIZE] = {
    0x5a,
    0x3b,
    0x74,
    0xb9
};

// Array of all the different aave selectors.
const uint8_t *const AAVE_SELECTORS[NUM_AAVE_SELECTORS] = {
    AAVE_DEPOSIT_SELECTOR,
    AAVE_BORROW_SELECTOR,
    AAVE_REPAY_SELECTOR,
    AAVE_WITHDRAW_SELECTOR,
    AAVE_SWAP_BORROW_RATE_SELECTOR,
    AAVE_SET_USAGE_AS_COLLATERAL_SELECTOR,
};

// Paraswap uses `0xeeeee` as a dummy address to represent ETH.
const uint8_t MOCK_ETH_ADDRESS[ADDRESS_LENGTH] = {0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee,
                                                      0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee,
                                                      0xee, 0xee, 0xee, 0xee, 0xee, 0xee};

// Used to indicate that the beneficiary should be the sender.
const uint8_t NULL_ETH_ADDRESS[ADDRESS_LENGTH] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

// Used to indicate that user will interact with the variable borrow rate mode
const uint8_t VARIABLE_BORROW_RATE_MODE_RAW[PARAMETER_LENGTH] = {
                                                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                  0x00, 0x00, 0x00, 0x01};

// Used to indicate that user will interact with the stable borrow rate mode
const uint8_t STABLE_BORROW_RATE_MODE_RAW[PARAMETER_LENGTH] = {
                                                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                  0x00, 0x00, 0x00, 0x02};

// Called once to init.
static void handle_init_contract(void *parameters) {
    ethPluginInitContract_t *msg = (ethPluginInitContract_t *) parameters;

    if (msg->interfaceVersion != ETH_PLUGIN_INTERFACE_VERSION_1) {
        msg->result = ETH_PLUGIN_RESULT_UNAVAILABLE;
        return;
    }

    if (msg->pluginContextLength < sizeof(aave_parameters_t)) {
        msg->result = ETH_PLUGIN_RESULT_ERROR;
        return;
    }

    aave_parameters_t *context = (aave_parameters_t *) msg->pluginContext;
    memset(context, 0, sizeof(*context));
    context->valid = 1;

    size_t i;
    PRINTF("CHECK SELECTORS\n");
    for (i = 0; i < NUM_AAVE_SELECTORS; i++) {
        if (memcmp((uint8_t *) PIC(AAVE_SELECTORS[i]), msg->selector, SELECTOR_SIZE) == 0) {
            context->selectorIndex = i;
            break;
        }
    }
    PRINTF("CHECK SELECTORS END\n");

    // Set `next_param` to be the first field we expect to parse.
    context->next_param = 0;

    // set contract address received to 0, because most part of the methods not use it
    memcpy(context->contract_address_received,
           NULL_ETH_ADDRESS,
           sizeof(context->contract_address_received));

    msg->result = ETH_PLUGIN_RESULT_OK;
}

static void handle_finalize(void *parameters) {
    ethPluginFinalize_t *msg = (ethPluginFinalize_t *) parameters;
    aave_parameters_t *context = (aave_parameters_t *) msg->pluginContext;
    PRINTF("eth2 plugin finalize\n");
    if (context->valid) {
        msg->numScreens = 2;

        // TODO: unmock in case we need more screens in some cases
//        if (context->selectorIndex == SIMPLE_SWAP || context->selectorIndex == SIMPLE_BUY)
//            if (strncmp(context->beneficiary, (char *) NULL_ETH_ADDRESS, ADDRESS_LENGTH) != 0) {
//                // An additional screen is required to display the `beneficiary` field.
//                msg->numScreens += 1;
//            }
        if (!ADDRESS_IS_ETH(context->contract_address_sent)) {
            // Address is not ETH so we will need to look up the token in the CAL.
            msg->tokenLookup1 = context->contract_address_sent;
            PRINTF("Setting address sent to: %.*H\n",
                   ADDRESS_LENGTH,
                   context->contract_address_sent);
        } else {
            msg->tokenLookup1 = NULL;
        }

        msg->tokenLookup2 = NULL;
        // TODO: unmock for swaps
//        if (!ADDRESS_IS_ETH(context->contract_address_received)) {
//            // Address is not ETH so we will need to look up the token in the CAL.
//            PRINTF("Setting address receiving to: %.*H\n",
//                   ADDRESS_LENGTH,
//                   context->contract_address_received);
//            msg->tokenLookup2 = context->contract_address_received;
//        } else {
//            msg->tokenLookup2 = NULL;
//        }

        msg->uiType = ETH_UI_TYPE_GENERIC;
        msg->result = ETH_PLUGIN_RESULT_OK;
    } else {
        PRINTF("Context not valid\n");
        msg->result = ETH_PLUGIN_RESULT_FALLBACK;
    }
}

static void handle_provide_token(void *parameters) {
    ethPluginProvideToken_t *msg = (ethPluginProvideToken_t *) parameters;
    aave_parameters_t *context = (aave_parameters_t *) msg->pluginContext;
    PRINTF("ФФМУ plugin provide token: 0x%p, 0x%p\n", msg->token1, msg->token2);

    if (ADDRESS_IS_ETH(context->contract_address_sent)) {
        context->decimals_sent = WEI_TO_ETHER;
        strncpy(context->ticker_sent, "ETH", sizeof(context->ticker_sent));
        context->tokens_found |= TOKEN_SENT_FOUND;
    } else if (msg->token1 != NULL) {
        context->decimals_sent = msg->token1->decimals;
        strncpy(context->ticker_sent, (char *) msg->token1->ticker, sizeof(context->ticker_sent));
        context->tokens_found |= TOKEN_SENT_FOUND;
    } else {
        // CAL did not find the token and token is not ETH.
        context->decimals_sent = DEFAULT_DECIMAL;
        strncpy(context->ticker_sent, DEFAULT_TICKER, sizeof(context->ticker_sent));
        // We will need an additional screen to display a warning message.
        msg->additionalScreens++;
    }

    if (!ADDRESS_IS_NULL(context->contract_address_received)) {
        if (ADDRESS_IS_ETH(context->contract_address_received)) {
            context->decimals_received = WEI_TO_ETHER;
            strncpy(context->ticker_received, "ETH", sizeof(context->ticker_received));
            context->tokens_found |= TOKEN_RECEIVED_FOUND;
        } else if (msg->token2 != NULL) {
            context->decimals_received = msg->token2->decimals;
            strncpy(context->ticker_received,
                    (char *) msg->token2->ticker,
                    sizeof(context->ticker_received));
            context->tokens_found |= TOKEN_RECEIVED_FOUND;
        } else {
            // CAL did not find the token and token is not ETH.
            context->decimals_received = DEFAULT_DECIMAL;
            strncpy(context->ticker_received, DEFAULT_TICKER, sizeof(context->ticker_sent));
            // We will need an additional screen to display a warning message.
            msg->additionalScreens++;
        }
    }

    msg->result = ETH_PLUGIN_RESULT_OK;
}

static void handle_query_contract_id(void *parameters) {
    ethQueryContractID_t *msg = (ethQueryContractID_t *) parameters;
    aave_parameters_t *context = (aave_parameters_t *) msg->pluginContext;

    strncpy(msg->name, PLUGIN_NAME, msg->nameLength);

    switch (context->selectorIndex) {
        case DEPOSIT:
            strncpy(msg->version, "Deposit", msg->versionLength);
            break;
        case BORROW:
            strncpy(msg->version, "Borrow", msg->versionLength);
            break;
        case REPAY:
            strncpy(msg->version, "Repay", msg->versionLength);
            break;
        case WITHDRAW:
            strncpy(msg->version, "Withdraw", msg->versionLength);
            break;
        case SET_USAGE_AS_COLLATERAL:
            strncpy(msg->version, "Set usage as collateral", msg->versionLength);
            break;
        case SWAP_BORROW_RATE:
            strncpy(msg->version, "Swap borrow rate", msg->versionLength);
            break;
        default:
            PRINTF("Selector Index :%d not supported\n", context->selectorIndex);
            msg->result = ETH_PLUGIN_RESULT_ERROR;
            return;
    }

    msg->result = ETH_PLUGIN_RESULT_OK;
}

void paraswap_plugin_call(int message, void *parameters) {
    PRINTF("ENTRYPOINT+++++++++++++++++++++++++++++++++++++++++++++++++==\n");
    switch (message) {
        case ETH_PLUGIN_INIT_CONTRACT:
            handle_init_contract(parameters);
            break;
        case ETH_PLUGIN_PROVIDE_PARAMETER:
            handle_provide_parameter(parameters);
            break;
        case ETH_PLUGIN_FINALIZE:
            handle_finalize(parameters);
            break;
        case ETH_PLUGIN_PROVIDE_TOKEN:
            handle_provide_token(parameters);
            break;
        case ETH_PLUGIN_QUERY_CONTRACT_ID:
            handle_query_contract_id(parameters);
            break;
        case ETH_PLUGIN_QUERY_CONTRACT_UI:
            handle_query_contract_ui(parameters);
            break;
        default:
            PRINTF("Unhandled message %d\n", message);
            break;
    }
}
