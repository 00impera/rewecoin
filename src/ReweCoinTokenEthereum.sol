// SPDX-License-Identifier: MIT
pragma solidity ^0.8.20;
import "@openzeppelin/contracts/token/ERC20/ERC20.sol";
import "@openzeppelin/contracts/access/Ownable.sol";

contract ReweCoinTokenEthereum is ERC20, Ownable {
    address public bridge;

    event Mint(address indexed to, uint256 amount);
    event Burn(address indexed from, uint256 amount);
    event BridgeUpdated(address indexed newBridge);

    constructor() ERC20("ReweCoin", "REWE") Ownable(msg.sender) {}

    modifier onlyBridge() {
        require(msg.sender == bridge, "Not bridge");
        _;
    }

    function mint(address to, uint256 amount) external onlyBridge {
        _mint(to, amount);
        emit Mint(to, amount);
    }

    function burn(address from, uint256 amount) external onlyBridge {
        _burn(from, amount);
        emit Burn(from, amount);
    }

    function setBridge(address newBridge) external onlyOwner {
        bridge = newBridge;
        emit BridgeUpdated(newBridge);
    }
}
