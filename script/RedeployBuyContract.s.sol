// SPDX-License-Identifier: MIT
pragma solidity ^0.8.20;

import {Script, console} from "forge-std/Script.sol";
import {SystemAccess} from "../src/SystemAccess.sol";
import {BuyContract} from "../src/BuyContract.sol";

contract RedeployBuyContractScript is Script {
    address constant SYSTEM_ACCESS      = 0xC7CFd53C0cC73168549aff18f3a0e5faF4113261;
    address constant REWE_TOKEN         = 0x330ACAD0591FEBBED0Fe7C28dde6704ac00Da9ae;
    address constant NFT_REWE           = 0x42c5b0512a8fe39358EA2850C012c3dc117E672E;
    address constant PLAYER_DATA        = 0xdAEC63459c3a0B6a1E9BA4d51074EDD7462f2b30;
    address constant POOL_MANAGER       = 0x1C0ee681DD3B7daA7330Ae4fD3814301A08BC17E;
    address constant USDC               = 0x754704Bc059F8C67012fEd69BC8A327a5aafb603;
    address constant USDT0              = 0xe7cd86e13AC4309349F30B3435a9d337750fC82D;
    address constant CHAINLINK_MON_USD  = 0xBcD78f76005B7515837af6b50c7C52BCf73822fb;

    address constant OLD_BUY_CONTRACT   = 0x008c095dc3aE2EE658Cd04b8C261Cc8a40E6854F;

    function run() external {
        uint256 deployerKey = vm.envUint("PRIVATE_KEY");
        vm.startBroadcast(deployerKey);

        BuyContract newBuy = new BuyContract(
            SYSTEM_ACCESS,
            REWE_TOKEN,
            NFT_REWE,
            PLAYER_DATA,
            POOL_MANAGER,
            USDC,
            USDT0,
            CHAINLINK_MON_USD
        );

        SystemAccess(SYSTEM_ACCESS).addSystem(address(newBuy));
        SystemAccess(SYSTEM_ACCESS).removeSystem(OLD_BUY_CONTRACT);

        vm.stopBroadcast();

        console.log("=== REDEPLOY BuyContract (fix zecimale USDC/USDT) ===");
        console.log("BuyContract NOU:  ", address(newBuy));
        console.log("BuyContract VECHI dezactivat:", OLD_BUY_CONTRACT);
    }
}
