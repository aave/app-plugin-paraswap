import Zemu from "@zondax/zemu";
import Eth from "@ledgerhq/hw-app-eth";
import { generate_plugin_config } from "./generate_plugin_config";
import { parseEther, parseUnits, RLP } from "ethers/utils";

const transactionUploadDelay = 60000;

const sim_options_generic = {
  logging: true,
  X11: true,
  startDelay: 5000,
  custom: "",
};

const Resolve = require("path").resolve;

const NANOS_ETH_PATH = Resolve("elfs/ethereum_nanos.elf");
const NANOX_ETH_PATH = Resolve("elfs/ethereum_nanox.elf");

const NANOS_PLUGIN_PATH = Resolve("elfs/paraswap_nanos.elf");
const NANOX_PLUGIN_PATH = Resolve("elfs/paraswap_nanox.elf");

const NANOS_PLUGIN = { Paraswap: NANOS_PLUGIN_PATH };
const NANOX_PLUGIN = { Paraswap: NANOX_PLUGIN_PATH };

const paraswapJSON = generate_plugin_config();

const SPECULOS_ADDRESS = "0xFE984369CE3919AA7BB4F431082D027B4F8ED70C";
const RANDOM_ADDRESS = "0xaaaabbbbccccddddeeeeffffgggghhhhiiiijjjj";

let genericTx = {
  nonce: Number(0),
  gasLimit: Number(21000),
  gasPrice: parseUnits("1", "gwei"),
  value: parseEther("1"),
  chainId: 1,
  to: RANDOM_ADDRESS,
  data: null,
};

const TIMEOUT = 1000000;

/**
 * Generates a serializedTransaction from a rawHexTransaction copy pasted from etherscan.
 * @param {string} rawTx Raw transaction
 * @returns {string} serializedTx
 */
function txFromEtherscan(rawTx) {
  // Remove 0x prefix
  rawTx = rawTx.slice(2);

  let txType = rawTx.slice(0, 2);
  if (txType == "02" || txType == "01") {
    // Remove "02" prefix
    rawTx = rawTx.slice(2);
  } else {
    txType = "";
  }

  let decoded = RLP.decode("0x" + rawTx);
  if (txType != "") {
    decoded = decoded.slice(0, decoded.length - 3); // remove v, r, s
  } else {
    decoded[decoded.length - 1] = "0x"; // empty
    decoded[decoded.length - 2] = "0x"; // empty
    decoded[decoded.length - 3] = "0x01"; // chainID 1
  }

  // Encode back the data, drop the '0x' prefix
  let encoded = RLP.encode(decoded).slice(2);

  // Don't forget to prepend the txtype
  return txType + encoded;
}

/**
 * Emulation of the device using zemu
 * @param {string} device name of the device to emulate (nanos, nanox)
 * @param {function} func
 * @param {boolean} signed the plugin is already signed 
 * @returns {Promise}
 */
function zemu(device, func, signed = false) {
  return async () => {
    jest.setTimeout(TIMEOUT);
    let eth_path;
    let plugin;
    let sim_options = sim_options_generic;

    if (device === "nanos") {
      eth_path = NANOS_ETH_PATH;
      plugin = NANOS_PLUGIN;
      sim_options.model = "nanos";
    } else {
      eth_path = NANOX_ETH_PATH;
      plugin = NANOX_PLUGIN;
      sim_options.model = "nanox";
    }

    const sim = new Zemu(eth_path, plugin);

    try {
      await sim.start(sim_options);
      const transport = await sim.getTransport();
      const eth = new Eth(transport);

      if(!signed){
        eth.setPluginsLoadConfig({
          baseURL: null,
          extraPlugins: paraswapJSON,
        });
      }
      await func(sim, eth);
    } finally {
      await sim.close();
    }
  };
}

/**
 * Process the trasaction through the full test process in interaction with the simulator
 * @param {Eth} eth Device to test (nanos, nanox)
 * @param {function} sim Zemu simulator
 * @param {int} steps Number of steps to push right button
 * @param {string} label directory against which the test snapshots must be checked.
 * @param {string} rawTxHex RawTransaction Hex to process
 */
async function processTransaction(eth, sim, steps, label, rawTxHex) {
  const serializedTx = txFromEtherscan(rawTxHex);
  let tx = eth.signTransaction("44'/60'/0'/0/0", serializedTx);

  await sim.waitUntilScreenIsNot(
    sim.getMainMenuSnapshot(),
    transactionUploadDelay
  );
  await sim.navigateAndCompareSnapshots(".", label, [steps, 0]);

  await tx;
}

/**
 * Function to execute test with the simulator
 * @param {Object} device Device including its name, its label, and the number of steps to process the use case
 * @param {string} contractName Name of the contract
 * @param {string} testLabel Name of the test case
 * @param {string} testDirSuffix Name of the folder suffix for snapshot comparison
 * @param {string} rawTxHex RawTx Hex to test
 * @param {boolean} signed The plugin is already signed and existing in Ledger database
 */
function processTest(device, contractName, testLabel, testDirSuffix, rawTxHex, signed ) {
  test(
    "[" + contractName + "] - " + device.label + " - " + testLabel,
    zemu(device.name, async (sim, eth) => {
      await processTransaction(
        eth,
        sim,
        device.steps,
        device.name + "_" + testDirSuffix,
        rawTxHex
      );
    },signed)
  );
}

module.exports = {
  processTest,
};
