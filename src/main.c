/*******************************************************************************
 *   Ethereum 2 Deposit Application
 *   (c) 2020 Ledger
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 ********************************************************************************/

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "os.h"
#include "cx.h"

#include "glyphs.h"

#include "paraswap_plugin.h"

void call_app_ethereum() {
    unsigned int libcall_params[3];
    libcall_params[0] = (unsigned int) "Ethereum";
    libcall_params[1] = 0x100;
    libcall_params[2] = RUN_APPLICATION;
    os_lib_call((unsigned int *) &libcall_params);
}

__attribute__((section(".boot"))) int main(int arg0) {
    // exit critical section
    __asm volatile("cpsie i");

    // ensure exception will work as planned
    os_boot();

    BEGIN_TRY {
        TRY {
            check_api_level(CX_COMPAT_APILEVEL);

            if (!arg0) {
                // called from dashboard, launch Ethereum app
                call_app_ethereum();
                return 0;
            } else {
                // regular call from ethereum
                unsigned int *args = (unsigned int *) arg0;

                if (args[0] != ETH_PLUGIN_CHECK_PRESENCE) {
                    paraswap_plugin_call(args[0], (void *) args[1]);
                }
                os_lib_end();
            }
        }
        FINALLY {
        }
    }
    END_TRY;

    return 0;
}
