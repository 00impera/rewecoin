// SPDX-License-Identifier: MIT
pragma solidity ^0.8.20;

import "@openzeppelin/contracts/access/Ownable.sol";
import "./SystemAccess.sol";
import "./ReweCoinToken.sol";

interface ILayerZeroEndpointV2 {
    function send(
        uint32 dstEid,
        bytes calldata message,
        bytes calldata options,
        address refundAddress
    ) external payable;
}

contract BridgeMonad is Ownable {

    SystemAccess public access;
    ReweCoinToken public rewe;

    ILayerZeroEndpointV2 public endpoint;

    uint32 public monadEid = 30132;     // example EID for Monad
    uint32 public ethereumEid = 30101;  // example EID for Ethereum

    event BridgeSend(address indexed user, uint256 amount, uint32 dstChain);
    event BridgeReceive(address indexed user, uint256 amount, uint32 srcChain);

    constructor(
        address _access,
        address _rewe,
        address _endpoint
    ) {
        access = SystemAccess(_access);
        rewe = ReweCoinToken(_rewe);
        endpoint = ILayerZeroEndpointV2(_endpoint);
    }

    modifier onlySystem() {
        require(access.isSystem(msg.sender), "Not system");
        _;
    }

    // ---------------------------------------------------------
    // SEND REWE TO ANOTHER CHAIN
    // ---------------------------------------------------------
    function bridgeToChain(uint32 dstChain, uint256 amount) external payable {
        require(amount > 0, "Invalid amount");

        // burn REWE on source chain
        rewe.burn(msg.sender, amount);

        // encode message
        bytes memory message = abi.encode(msg.sender, amount);

        // send via LayerZero
        endpoint.send(
            dstChain,
            message,
            bytes(""),       // default options
            msg.sender       // refund address
        );

        emit BridgeSend(msg.sender, amount, dstChain);
    }

    // ---------------------------------------------------------
    // RECEIVE REWE FROM ANOTHER CHAIN
    // CALLED BY LAYERZERO EXECUTOR
    // ---------------------------------------------------------
    function lzReceive(
        bytes calldata message,
        uint32 srcChain,
        address /*executor*/,
        bytes calldata /*extraData*/
    ) external {
        require(msg.sender == address(endpoint), "Invalid caller");

        (address user, uint256 amount) = abi.decode(message, (address, uint256));

        // mint REWE on destination chain
        rewe.mint(user, amount);

        emit BridgeReceive(user, amount, srcChain);
    }

    // ---------------------------------------------------------
    // OWNER SETTINGS
    // ---------------------------------------------------------
    function updateAccess(address newAccess) external onlyOwner {
        access = SystemAccess(newAccess);
    }

    function updateEndpoint(address newEndpoint) external onlyOwner {
        endpoint = ILayerZeroEndpointV2(newEndpoint);
    }

    function updateChainIds(uint32 newMonad, uint32 newEthereum) external onlyOwner {
        monadEid = newMonad;
        ethereumEid = newEthereum;
    }
}
