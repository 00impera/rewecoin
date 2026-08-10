// SPDX-License-Identifier: MIT
pragma solidity ^0.8.20;

import "@openzeppelin/contracts/access/Ownable.sol";
import "./SystemAccess.sol";

contract Leaderboard is Ownable {

    SystemAccess public access;

    struct Entry {
        address user;
        uint256 usdValue;
    }

    // poolId => top 3 entries
    mapping(uint256 => Entry[3]) public top3;

    event LeaderboardUpdated(uint256 indexed poolId, address indexed user, uint256 usdValue);

    constructor(address _access) {
        access = SystemAccess(_access);
    }

    modifier onlySystem() {
        require(access.isSystem(msg.sender), "Not system");
        _;
    }

    function updateLeaderboard(
        uint256 poolId,
        address user,
        uint256 usdValue
    ) external onlySystem {

        Entry[3] storage board = top3[poolId];

        // If user already in top 3, update value
        for (uint256 i = 0; i < 3; i++) {
            if (board[i].user == user) {
                board[i].usdValue = usdValue;
                _sort(board);
                emit LeaderboardUpdated(poolId, user, usdValue);
                return;
            }
        }

        // If user not in top 3, check if qualifies
        for (uint256 i = 0; i < 3; i++) {
            if (usdValue > board[i].usdValue) {
                // shift down
                for (uint256 j = 2; j > i; j--) {
                    board[j] = board[j - 1];
                }
                board[i] = Entry(user, usdValue);
                _sort(board);
                emit LeaderboardUpdated(poolId, user, usdValue);
                return;
            }
        }
    }

    function _sort(Entry[3] storage board) internal {
        // simple bubble sort for 3 entries
        for (uint256 i = 0; i < 2; i++) {
            for (uint256 j = i + 1; j < 3; j++) {
                if (board[j].usdValue > board[i].usdValue) {
                    Entry memory temp = board[i];
                    board[i] = board[j];
                    board[j] = temp;
                }
            }
        }
    }

    function getTop3(uint256 poolId)
        external
        view
        returns (Entry[3] memory)
    {
        return top3[poolId];
    }

    function updateAccess(address newAccess) external onlyOwner {
        access = SystemAccess(newAccess);
    }
}
