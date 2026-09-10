// SPDX-License-Identifier: MIT
pragma solidity ^0.8.20;

import {Script, console} from "forge-std/Script.sol";
import {SystemAccess} from "../src/SystemAccess.sol";
import {VaultManager} from "../src/VaultManager.sol";

contract DeployVaultManagerScript is Script {
    address constant SYSTEM_ACCESS = 0xC7CFd53C0cC73168549aff18f3a0e5faF4113261;
    address constant REWE_TOKEN     = 0x330ACAD0591FEBBED0Fe7C28dde6704ac00Da9ae;

    address constant PAIR_REWE_USDC = 0x600e270775f26847C67241bEcf6F7C92718A3876;
    address constant PAIR_REWE_MON  = 0x845bcAad6F6C26EeE1DE0086fBAe7A13fE1Cb654;
    address constant PAIR_REWE_USDT = 0x09473ddEA01EFC6fBEA5d05636c309a949b6CA1C;

    uint256 constant INITIAL_REWARD_PER_SECOND = 100000000000000;

    function run() external {
        uint256 deployerKey = vm.envUint("PRIVATE_KEY");
        vm.startBroadcast(deployerKey);

        VaultManager vault = new VaultManager(REWE_TOKEN, INITIAL_REWARD_PER_SECOND);

        SystemAccess(SYSTEM_ACCESS).addSystem(address(vault));

        vault.add(100, PAIR_REWE_USDC);
        vault.add(100, PAIR_REWE_MON);
        vault.add(100, PAIR_REWE_USDT);

        vm.stopBroadcast();

        console.log("=== DEPLOY VaultManager ===");
        console.log("VaultManager:", address(vault));
        console.log("Pool 0 (REWE/USDC):", PAIR_REWE_USDC);
        console.log("Pool 1 (REWE/MON): ", PAIR_REWE_MON);
        console.log("Pool 2 (REWE/USDT):", PAIR_REWE_USDT);
    }
}
