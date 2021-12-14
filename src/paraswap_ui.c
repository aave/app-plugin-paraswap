#include "paraswap_plugin.h"

// Prepend `dest` with `ticker`.
// Dest must be big enough to hold `ticker` + `dest` + `\0`.
static void prepend_ticker(char *dest, uint8_t destsize, char *ticker) {
    if (dest == NULL || ticker == NULL) {
        THROW(0x6503);
    }
    uint8_t ticker_len = strlen(ticker);
    uint8_t dest_len = strlen(dest);

    if (dest_len + ticker_len >= destsize) {
        THROW(0x6503);
    }

    // Right shift the string by `ticker_len` bytes.
    while (dest_len != 0) {
        dest[dest_len + ticker_len] = dest[dest_len];  // First iteration will copy the \0
        dest_len--;
    }
    // Don't forget to null terminate the string.
    dest[ticker_len] = dest[0];

    // Copy the ticker to the beginning of the string.
    memcpy(dest, ticker, ticker_len);
}

// Set UI for the "Send" screen.
static void set_send_ui(ethQueryContractUI_t *msg, aave_parameters_t *context) {
//    switch (context->selectorIndex) {
//        case SWAP_ON_UNI_FORK:
//        case SWAP_ON_UNI:
//        case SIMPLE_SWAP:
//        case MEGA_SWAP:
//        case MULTI_SWAP:
//            strncpy(msg->title, "Send", msg->titleLength);
//            break;
//        case BUY_ON_UNI_FORK:
//        case BUY_ON_UNI:
//        case BUY:
//        case SIMPLE_BUY:
//            strncpy(msg->title, "Send Max", msg->titleLength);
//            break;
//        default:
//            PRINTF("Unhandled selector Index: %d\n", context->selectorIndex);
//            msg->result = ETH_PLUGIN_RESULT_ERROR;
//            return;
//    }

    adjustDecimals((char *) context->amount_sent,
                   strnlen((char *) context->amount_sent, sizeof(context->amount_sent)),
                   msg->msg,
                   msg->msgLength,
                   context->decimals_sent);

    prepend_ticker(msg->msg, msg->msgLength, context->ticker_sent);
}

// Set UI for "Receive" screen.
static void set_receive_ui(ethQueryContractUI_t *msg, aave_parameters_t *context) {
//    switch (context->selectorIndex) {
//        case SWAP_ON_UNI_FORK:
//        case SWAP_ON_UNI:
//        case SIMPLE_SWAP:
//        case MEGA_SWAP:
//        case MULTI_SWAP:
//            strncpy(msg->title, "Receive Min", msg->titleLength);
//            break;
//        case BUY_ON_UNI_FORK:
//        case BUY_ON_UNI:
//        case BUY:
//        case SIMPLE_BUY:
//            strncpy(msg->title, "Receive", msg->titleLength);
//            break;
//        default:
//            PRINTF("Unhandled selector Index: %d\n", context->selectorIndex);
//            msg->result = ETH_PLUGIN_RESULT_ERROR;
//            return;
//    }

    adjustDecimals((char *) context->amount_received,
                   strnlen((char *) context->amount_received, sizeof(context->amount_received)),
                   msg->msg,
                   msg->msgLength,
                   context->decimals_received);

    prepend_ticker(msg->msg, msg->msgLength, context->ticker_received);
}

// Set UI for "Beneficiary" screen.
static void set_beneficiary_ui(ethQueryContractUI_t *msg, aave_parameters_t *context) {
    strncpy(msg->title, "Beneficiary", msg->titleLength);

    msg->msg[0] = '0';
    msg->msg[1] = 'x';

    chain_config_t chainConfig = {0};

    getEthAddressStringFromBinary((uint8_t *) context->beneficiary,
                                  (uint8_t *) msg->msg + 2,
                                  msg->pluginSharedRW->sha3,
                                  &chainConfig);
}

// Set UI for "Warning" screen.
static void set_warning_ui(ethQueryContractUI_t *msg,
                           aave_parameters_t *context __attribute__((unused))) {
    strncpy(msg->title, "WARNING", msg->titleLength);
    strncpy(msg->msg, "Unknown token", msg->msgLength);
}

// Helper function that returns the enum corresponding to the screen that should be displayed.
static screens_t get_screen(ethQueryContractUI_t *msg, aave_parameters_t *context) {
    uint8_t index = msg->screenIndex;

    bool token_sent_found = context->tokens_found & TOKEN_SENT_FOUND;
    bool token_received_found = context->tokens_found & TOKEN_RECEIVED_FOUND;

    bool token_received_needed = ADDRESS_IS_NULL(context->contract_address_received);

    if (index == 0) {
        switch (context->selectorIndex) {
            case DEPOSIT:
            case BORROW:
            case WITHDRAW:
            case REPAY : {
                if (token_sent_found) {
                    return SEND_SCREEN;
                }
                return WARN_SCREEN;
            }
            case SET_USAGE_AS_COLLATERAL: {
                // TODO: implement
                return ERROR;
            }
            case SWAP_BORROW_RATE: {
                // TODO: implement
                return ERROR;
            }
            default: {
                return ERROR;
            }
        }
    } else if (index == 1) {
        switch (context->selectorIndex) {
            case DEPOSIT:
            case BORROW:
            case WITHDRAW:
            case REPAY : {
                if (token_sent_found) {
                    return BENEFICIARY_SCREEN;
                }
                return SEND_SCREEN;
            }
            case SET_USAGE_AS_COLLATERAL: {
                // TODO: implement
                return ERROR;
            }
            case SWAP_BORROW_RATE: {
                // TODO: implement
                return ERROR;
            }
            default: {
                return ERROR;
            }
        }
    } else if (index == 2) {
        switch (context->selectorIndex) {
            case DEPOSIT:
            case BORROW:
            case WITHDRAW:
            case REPAY : {
                if (token_sent_found) {
                    return ERROR;
                }
                return BENEFICIARY_SCREEN;
            }
            case SET_USAGE_AS_COLLATERAL: {
                // TODO: implement
                return ERROR;
            }
            case SWAP_BORROW_RATE: {
                // TODO: implement
                return ERROR;
            }
            default: {
                return ERROR;
            }
        }
    }
//    else if (index == 3) {
//        if (both_tokens_found) {
//            return ERROR;
//        } else if (both_tokens_not_found) {
//            return RECEIVE_SCREEN;
//        } else {
//            return BENEFICIARY_SCREEN;
//        }
//    } else if (index == 4) {
//        if (both_tokens_not_found) {
//            return BENEFICIARY_SCREEN;
//        } else {
//            return ERROR;
//        }
//    }
    return ERROR;
}

void handle_query_contract_ui(void *parameters) {
    ethQueryContractUI_t *msg = (ethQueryContractUI_t *) parameters;
    aave_parameters_t *context = (aave_parameters_t *) msg->pluginContext;

    memset(msg->title, 0, msg->titleLength);
    memset(msg->msg, 0, msg->msgLength);
    msg->result = ETH_PLUGIN_RESULT_OK;

    screens_t screen = get_screen(msg, context);
    switch (screen) {
        case SEND_SCREEN:
            set_send_ui(msg, context);
            break;
        case RECEIVE_SCREEN:
            set_receive_ui(msg, context);
            break;
        case BENEFICIARY_SCREEN:
            set_beneficiary_ui(msg, context);
            break;
        case WARN_SCREEN:
            set_warning_ui(msg, context);
            break;
        default:
            PRINTF("Received an invalid screenIndex\n");
            msg->result = ETH_PLUGIN_RESULT_ERROR;
            return;
    }
}
