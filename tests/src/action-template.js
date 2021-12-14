import "core-js/stable";
import "regenerator-runtime/runtime";
import Eth from "@ledgerhq/hw-app-eth";
import Zemu from "@zondax/zemu";

const sim_options = {
  model: "nanos",
  logging: true,
  start_delay: 2000,
  X11: true,
  custom: '',
};
const Resolve = require("path").resolve;
const APP_PATH = Resolve("elfs/ethereum.elf");

const AAVE_LIB = { Aave: Resolve("elfs/aave.elf") };

export const testTemplate = async (name, txToSign, handle) => {
  test(name, async () => {
    jest.setTimeout(100000);
    const sim = new Zemu(APP_PATH, AAVE_LIB);
    try {
      await sim.start(sim_options);

      let transport = await sim.getTransport();
      const eth = new Eth(transport);

      eth.signTransaction("44'/60'/0'/0/0", txToSign);

      await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
      await handle(sim);
      await sim.clickBoth(); // This is not correct yet

    } finally {
      await sim.close();
    }
  });
}