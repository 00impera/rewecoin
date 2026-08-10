// SPDX-License-Identifier: MIT
pragma solidity ^0.8.20;

import "@openzeppelin/contracts/token/ERC20/ERC20.sol";
import "@openzeppelin/contracts/access/Ownable.sol";
import "./SystemAccess.sol";

contract ReweCoinToken is ERC20, Ownable {

    SystemAccess public access;

    event Mint(address indexed to, uint256 amount);
    event Burn(address indexed from, uint256 amount);

    constructor(address _access) ERC20("ReweCoin", "REWE") {
        access = SystemAccess(_access);
    }

    modifier onlySystem() {
        require(access.isSystem(msg.sender), "Not system");
        _;
    }

    function mint(address to, uint256 amount) external onlySystem {
        _mint(to, amount);
        emit Mint(to, amount);
    }

    function burn(address from, uint256 amount) external onlySystem {
        _burn(from, amount);
        emit Burn(from, amount);
    }

    function updateAccess(address newAccess) external onlyOwner {
        access = SystemAccess(newAccess);
    }
}
