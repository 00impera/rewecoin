// SPDX-License-Identifier: MIT
pragma solidity ^0.8.20;

import "@openzeppelin/contracts/access/Ownable.sol";

contract SystemAccess is Ownable {

    mapping(address => bool) public isSystem;

    event SystemAdded(address indexed system);
    event SystemRemoved(address indexed system);

    modifier onlySystem() {
        require(isSystem[msg.sender], "Not system");
        _;
    }

    function addSystem(address system) external onlyOwner {
        isSystem[system] = true;
        emit SystemAdded(system);
    }

    function removeSystem(address system) external onlyOwner {
        isSystem[system] = false;
        emit SystemRemoved(system);
    }

    function isSystemAddress(address system) external view returns (bool) {
        return isSystem[system];
    }
}
