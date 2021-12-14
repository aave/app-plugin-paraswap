#include "paraswap_plugin.h"

// Store the amount sent in the form of a string, without any ticker or decimals. These will be
// added when displaying.
static void handle_amount_from(ethPluginProvideParameter_t *msg, aave_parameters_t *context) {
    memset(context->amount_sent, 0, sizeof(context->amount_sent));

    PRINTF("%.*H", sizeof(context->amount_sent), msg->parameter);
    // Convert to string.
    amountToString(msg->parameter,
                   PARAMETER_LENGTH,
                   0,   // No decimals
                   "",  // No ticker
                   (char *) context->amount_sent,
                   sizeof(context->amount_sent));
    PRINTF("AMOUNT SENT: %s\n", context->amount_sent);
}

static void handle_asset_from(ethPluginProvideParameter_t *msg, aave_parameters_t *context) {
    memset(context->contract_address_sent, 0, sizeof(context->contract_address_sent));
    memcpy(context->contract_address_sent,
           &msg->parameter[PARAMETER_LENGTH - ADDRESS_LENGTH],
           sizeof(context->contract_address_sent));
    PRINTF("ASSET FROM: %.*H\n", ADDRESS_LENGTH, context->contract_address_sent);
}

static void handle_asset_to(ethPluginProvideParameter_t *msg, aave_parameters_t *context) {
    memset(context->contract_address_received, 0, sizeof(context->contract_address_received));
    memcpy(context->contract_address_received,
           &msg->parameter[PARAMETER_LENGTH - ADDRESS_LENGTH],
           sizeof(context->contract_address_received));
    PRINTF("ASSET TO: %.*H\n", ADDRESS_LENGTH, context->contract_address_received);
}

static void handle_beneficiary(ethPluginProvideParameter_t *msg, aave_parameters_t *context) {
    memset(context->beneficiary, 0, sizeof(context->beneficiary));
    memcpy(context->beneficiary,
           &msg->parameter[PARAMETER_LENGTH - ADDRESS_LENGTH],
           sizeof(context->beneficiary));
    PRINTF("BENEFICIARY: %.*H\n", ADDRESS_LENGTH, context->beneficiary);
}

static void handle_interest_rate_mode(ethPluginProvideParameter_t *msg, aave_parameters_t *context) {

    if(BORROW_RATE_MODE_IS_VARIABLE(msg->parameter)) {
        context->interest_rate_mode = VARIABLE;
        PRINTF("VARIABLE BORROW RATE");
    } else if(BORROW_RATE_MODE_IS_STABLE(msg->parameter)) {
        context->interest_rate_mode = STABLE;
        PRINTF("STABLE BORROW RATE");
    } else {
        PRINTF("NO BORROW RATE SELECTED");
        msg->result = ETH_PLUGIN_RESULT_ERROR;
    }
}




// Store the amount sent in the form of a string, without any ticker or decimals. These will be
// added when displaying.
static void handle_amount_sent(ethPluginProvideParameter_t *msg, aave_parameters_t *context) {
    memset(context->amount_sent, 0, sizeof(context->amount_sent));

    // Convert to string.
    amountToString(msg->parameter,
                   PARAMETER_LENGTH,
                   0,
                   "",
                   (char *) context->amount_sent,
                   sizeof(context->amount_sent));
    PRINTF("AMOUNT SENT: %s\n", context->amount_sent);
}

// Store the amount received in the form of a string, without any ticker or decimals. These will be
// added when displaying.
static void handle_amount_received(ethPluginProvideParameter_t *msg,
                                   aave_parameters_t *context) {
    memset(context->amount_received, 0, sizeof(context->amount_received));

    // Convert to string.
    amountToString(msg->parameter,
                   PARAMETER_LENGTH,
                   0,   // No decimals
                   "",  // No ticker
                   (char *) context->amount_received,
                   sizeof(context->amount_received));
    PRINTF("AMOUNT RECEIVED: %s\n", context->amount_received);
}

static void handle_token_sent(ethPluginProvideParameter_t *msg, aave_parameters_t *context) {
    memset(context->contract_address_sent, 0, sizeof(context->contract_address_sent));
    memcpy(context->contract_address_sent,
           &msg->parameter[PARAMETER_LENGTH - ADDRESS_LENGTH],
           sizeof(context->contract_address_sent));
    PRINTF("TOKEN SENT: %.*H\n", ADDRESS_LENGTH, context->contract_address_sent);
}

static void handle_token_received(ethPluginProvideParameter_t *msg,
                                  aave_parameters_t *context) {
    memset(context->contract_address_received, 0, sizeof(context->contract_address_received));
    memcpy(context->contract_address_received,
           &msg->parameter[PARAMETER_LENGTH - ADDRESS_LENGTH],
           sizeof(context->contract_address_received));
    PRINTF("TOKEN RECEIVED: %.*H\n", ADDRESS_LENGTH, context->contract_address_received);
}

static void handle_deposit_and_withdraw(ethPluginProvideParameter_t *msg,
                                     aave_parameters_t *context) {
    switch (context->next_param) {
        case ASSET:
            handle_asset_from(msg, context);
            context->next_param = AMOUNT;
            break;
        case AMOUNT:
            handle_amount_from(msg, context);
            context->next_param = ON_BEHALF_OF_DEPOSIT;
            break;
        case ON_BEHALF_OF_DEPOSIT:  // beneficiary
            handle_beneficiary(msg, context);
            context->next_param = REFERRAL_CODE;
            break;
        case REFERRAL_CODE:
            context->next_param = NONE;
            break;
        case NONE:
            break;
        default:
            PRINTF("Unsupported param\n");
            msg->result = ETH_PLUGIN_RESULT_ERROR;
            break;
    }
}

static void handle_borrow(ethPluginProvideParameter_t *msg,
                                     aave_parameters_t *context) {
    switch (context->next_param) {
        case ASSET:
            handle_asset_from(msg, context);
            context->next_param = AMOUNT;
            break;
        case AMOUNT:
            handle_asset_from(msg, context);
            context->next_param = INTEREST_RATE_MODE;
            break;
        case INTEREST_RATE_MODE:
            handle_interest_rate_mode(msg, context);
            context->next_param = REFERRAL_CODE;
            break;
        case REFERRAL_CODE:
            context->next_param = ON_BEHALF_OF_BORROW;
            break;
            //TODO: change, beneficiary not the best name in the case of borrow on behalf
        case ON_BEHALF_OF_BORROW:  // beneficiary
            handle_beneficiary(msg, context);
            context->next_param = NONE;
            break;
        case NONE:
            break;
        default:
            PRINTF("Unsupported param\n");
            msg->result = ETH_PLUGIN_RESULT_ERROR;
            break;
    }
}

static void handle_repay(ethPluginProvideParameter_t *msg,
                                     aave_parameters_t *context) {
    switch (context->next_param) {
        case ASSET:
            handle_asset_from(msg, context);
            context->next_param = AMOUNT;
            break;
        case AMOUNT:
            handle_amount_from(msg, context);
            context->next_param = INTEREST_RATE_MODE;
            break;
        case INTEREST_RATE_MODE:
            handle_interest_rate_mode(msg, context);
            context->next_param = ON_BEHALF_OF_REPAY;
            break;
        case ON_BEHALF_OF_REPAY:  // beneficiary
            handle_beneficiary(msg, context);
            context->next_param = NONE;
            break;
        case NONE:
            break;
        default:
            PRINTF("Unsupported param\n");
            msg->result = ETH_PLUGIN_RESULT_ERROR;
            break;
    }
}

static void handle_swap_borrow_rate_mode(ethPluginProvideParameter_t *msg,
                                     aave_parameters_t *context) {
    switch (context->next_param) {
        case ASSET:
            handle_asset_from(msg, context);
            context->next_param = AMOUNT;
            break;
        case INTEREST_RATE_MODE_SWAP:
            handle_interest_rate_mode(msg, context);
            context->next_param = NONE;
            break;
        case NONE:
            break;
        default:
            PRINTF("Unsupported param\n");
            msg->result = ETH_PLUGIN_RESULT_ERROR;
            break;
    }
}

static void handle_set_usage_as_collateral(ethPluginProvideParameter_t *msg,
                                     aave_parameters_t *context) {
    switch (context->next_param) {
        case ASSET:
            handle_asset_from(msg, context);
            context->next_param = AMOUNT;
            break;
        case USAGE_AS_COLLATERAL_MODE:
            // TODO: to be defined
//            handle_interest_rate_mode(msg, context);
            context->next_param = NONE;
            break;
        case NONE:
            break;
        default:
            PRINTF("Unsupported param\n");
            msg->result = ETH_PLUGIN_RESULT_ERROR;
            break;
    }
}

void handle_provide_parameter(void *parameters) {
    ethPluginProvideParameter_t *msg = (ethPluginProvideParameter_t *) parameters;
    aave_parameters_t *context = (aave_parameters_t *) msg->pluginContext;
    PRINTF("eth2 plugin provide parameter %d %.*H\n",
           msg->parameterOffset,
           PARAMETER_LENGTH,
           msg->parameter);

    msg->result = ETH_PLUGIN_RESULT_OK;

    switch (context->selectorIndex) {
        case DEPOSIT:
        case WITHDRAW: {
            handle_deposit_and_withdraw(msg, context);
            break;
        }
        case BORROW:{
            handle_borrow(msg, context);
            break;
        }
        case REPAY:{
            handle_repay(msg, context);
            break;
        }
        case SWAP_BORROW_RATE:{
            handle_swap_borrow_rate_mode(msg, context);
            break;
        }
        case SET_USAGE_AS_COLLATERAL:{
            handle_set_usage_as_collateral(msg, context);
            break;
        }
        default:
            PRINTF("Selector Index %d not supported\n", context->selectorIndex);
            msg->result = ETH_PLUGIN_RESULT_ERROR;
            break;
    }
}