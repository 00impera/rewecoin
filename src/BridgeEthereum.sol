// SPDX-License-Identifier: MIT
pragma solidity ^0.8.20;

import { OApp, Origin, MessagingFee } from "@layerzerolabs/oapp-evm/contracts/oapp/OApp.sol";
import "./ReweCoinTokenEthereum.sol";

contract BridgeEthereum is OApp {
    ReweCoinTokenEthereum public rewe;

    event BridgeSend(address indexed user, uint256 amount, uint32 dstEid);
    event BridgeReceive(address indexed user, uint256 amount, uint32 srcEid);

    constructor(
        address _endpoint,
        address _owner,
        address _rewe
    ) OApp(_endpoint, _owner) Ownable(_owner) {
        rewe = ReweCoinTokenEthereum(_rewe);
    }

    function quoteBridge(
        uint32 dstEid,
        address user,
        uint256 amount,
        bytes calldata options
    ) external view returns (MessagingFee memory fee) {
        bytes memory payload = abi.encode(user, amount);
        fee = _quote(dstEid, payload, options, false);
    }

    function bridgeToChain(uint32 dstEid, uint256 amount, bytes calldata options) external payable {
        require(amount > 0, "Invalid amount");
        rewe.burn(msg.sender, amount);
        bytes memory payload = abi.encode(msg.sender, amount);
        _lzSend(dstEid, payload, options, MessagingFee(msg.value, 0), payable(msg.sender));
        emit BridgeSend(msg.sender, amount, dstEid);
    }

    function _lzReceive(
        Origin calldata _origin,
        bytes32,
        bytes calldata _message,
        address,
        bytes calldata
    ) internal override {
        (address user, uint256 amount) = abi.decode(_message, (address, uint256));
        rewe.mint(user, amount);
        emit BridgeReceive(user, amount, _origin.srcEid);
    }
}
