// SPDX-License-Identifier: MIT
pragma solidity ^0.8.20;

import "@openzeppelin/contracts/access/Ownable.sol";
import "./SystemAccess.sol";

contract PlayerData is Ownable {

    SystemAccess public access;

    struct Player {
        uint256 totalUsd;
        uint256 totalRewe;
        uint256 points;
        uint256[] nftIds;
        uint256[] pools;
    }

    mapping(address => Player) public players;

    event USDAdded(address indexed user, uint256 amount);
    event REWEAdded(address indexed user, uint256 amount);
    event PointsAdded(address indexed user, uint256 amount);
    event NFTAdded(address indexed user, uint256 nftId);
    event PoolJoined(address indexed user, uint256 poolId);

    constructor(address _access) {
        access = SystemAccess(_access);
    }

    modifier onlySystem() {
        require(access.isSystem(msg.sender), "Not system");
        _;
    }

    function addUsd(address user, uint256 amount) external onlySystem {
        players[user].totalUsd += amount;
        emit USDAdded(user, amount);
    }

    function addRewe(address user, uint256 amount) external onlySystem {
        players[user].totalRewe += amount;
        emit REWEAdded(user, amount);
    }

    function addPoints(address user, uint256 amount) external onlySystem {
        players[user].points += amount;
        emit PointsAdded(user, amount);
    }

    function addNFT(address user, uint256 nftId) external onlySystem {
        players[user].nftIds.push(nftId);
        emit NFTAdded(user, nftId);
    }

    function joinPool(address user, uint256 poolId) external onlySystem {
        players[user].pools.push(poolId);
        emit PoolJoined(user, poolId);
    }

    function getPlayer(address user)
        external
        view
        returns (
            uint256 totalUsd,
            uint256 totalRewe,
            uint256 points,
            uint256[] memory nftIds,
            uint256[] memory pools
        )
    {
        Player storage p = players[user];
        return (p.totalUsd, p.totalRewe, p.points, p.nftIds, p.pools);
    }

    function updateAccess(address newAccess) external onlyOwner {
        access = SystemAccess(newAccess);
    }
}
