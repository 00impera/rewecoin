// SPDX-License-Identifier: MIT
pragma solidity ^0.8.20;

import "@openzeppelin/contracts/token/ERC20/IERC20.sol";
import "@openzeppelin/contracts/token/ERC20/utils/SafeERC20.sol";
import "./AmmFactory.sol";
import "./AmmPair.sol";
import "./WMON.sol";

contract AmmRouter {
    using SafeERC20 for IERC20;

    AmmFactory public immutable factory;
    WMON public immutable wmon;

    constructor(address _factory, address _wmon) {
        factory = AmmFactory(_factory);
        wmon = WMON(payable(_wmon));
    }

    modifier ensure(uint256 deadline) {
        require(block.timestamp <= deadline, "AmmRouter: expired");
        _;
    }

    receive() external payable {}

    function _sortTokens(address tokenA, address tokenB) internal pure returns (address token0, address token1) {
        (token0, token1) = tokenA < tokenB ? (tokenA, tokenB) : (tokenB, tokenA);
    }

    function _pairFor(address tokenA, address tokenB) internal view returns (address pair) {
        pair = factory.getPair(tokenA, tokenB);
        require(pair != address(0), "AmmRouter: pair does not exist");
    }

    function getAmountOut(uint256 amountIn, uint256 reserveIn, uint256 reserveOut) public pure returns (uint256) {
        require(amountIn > 0, "AmmRouter: insufficient input amount");
        require(reserveIn > 0 && reserveOut > 0, "AmmRouter: insufficient liquidity");
        uint256 amountInWithFee = amountIn * 997;
        uint256 numerator = amountInWithFee * reserveOut;
        uint256 denominator = reserveIn * 1000 + amountInWithFee;
        return numerator / denominator;
    }

    function quote(address tokenIn, address tokenOut, uint256 amountIn) external view returns (uint256 amountOut) {
        address pair = _pairFor(tokenIn, tokenOut);
        (address token0, ) = _sortTokens(tokenIn, tokenOut);
        (uint112 r0, uint112 r1) = AmmPair(pair).getReserves();
        (uint256 reserveIn, uint256 reserveOut) = tokenIn == token0 ? (uint256(r0), uint256(r1)) : (uint256(r1), uint256(r0));
        amountOut = getAmountOut(amountIn, reserveIn, reserveOut);
    }

    function addLiquidity(
        address tokenA,
        address tokenB,
        uint256 amountADesired,
        uint256 amountBDesired,
        uint256 amountAMin,
        uint256 amountBMin,
        address to,
        uint256 deadline
    ) external ensure(deadline) returns (uint256 amountA, uint256 amountB, uint256 liquidity) {
        address pair = factory.getPair(tokenA, tokenB);
        if (pair == address(0)) {
            pair = factory.createPair(tokenA, tokenB);
        }

        (uint112 r0, uint112 r1) = AmmPair(pair).getReserves();
        (address token0, ) = _sortTokens(tokenA, tokenB);
        (uint256 reserveA, uint256 reserveB) = tokenA == token0 ? (uint256(r0), uint256(r1)) : (uint256(r1), uint256(r0));

        if (reserveA == 0 && reserveB == 0) {
            (amountA, amountB) = (amountADesired, amountBDesired);
        } else {
            uint256 amountBOptimal = (amountADesired * reserveB) / reserveA;
            if (amountBOptimal <= amountBDesired) {
                require(amountBOptimal >= amountBMin, "AmmRouter: insufficient B amount");
                (amountA, amountB) = (amountADesired, amountBOptimal);
            } else {
                uint256 amountAOptimal = (amountBDesired * reserveA) / reserveB;
                require(amountAOptimal >= amountAMin, "AmmRouter: insufficient A amount");
                (amountA, amountB) = (amountAOptimal, amountBDesired);
            }
        }

        IERC20(tokenA).safeTransferFrom(msg.sender, pair, amountA);
        IERC20(tokenB).safeTransferFrom(msg.sender, pair, amountB);
        liquidity = AmmPair(pair).mint(to);
    }

    function addLiquidityMON(
        address token,
        uint256 amountTokenDesired,
        uint256 amountTokenMin,
        uint256 amountMONMin,
        address to,
        uint256 deadline
    ) external payable ensure(deadline) returns (uint256 amountToken, uint256 amountMON, uint256 liquidity) {
        address pair = factory.getPair(token, address(wmon));
        if (pair == address(0)) {
            pair = factory.createPair(token, address(wmon));
        }

        (uint112 r0, uint112 r1) = AmmPair(pair).getReserves();
        (address token0, ) = _sortTokens(token, address(wmon));
        (uint256 reserveToken, uint256 reserveMON) = token == token0 ? (uint256(r0), uint256(r1)) : (uint256(r1), uint256(r0));

        if (reserveToken == 0 && reserveMON == 0) {
            (amountToken, amountMON) = (amountTokenDesired, msg.value);
        } else {
            uint256 amountMONOptimal = (amountTokenDesired * reserveMON) / reserveToken;
            if (amountMONOptimal <= msg.value) {
                require(amountMONOptimal >= amountMONMin, "AmmRouter: insufficient MON amount");
                (amountToken, amountMON) = (amountTokenDesired, amountMONOptimal);
            } else {
                uint256 amountTokenOptimal = (msg.value * reserveToken) / reserveMON;
                require(amountTokenOptimal >= amountTokenMin, "AmmRouter: insufficient token amount");
                (amountToken, amountMON) = (amountTokenOptimal, msg.value);
            }
        }

        IERC20(token).safeTransferFrom(msg.sender, pair, amountToken);
        wmon.deposit{value: amountMON}();
        require(wmon.transfer(pair, amountMON), "AmmRouter: WMON transfer failed");
        liquidity = AmmPair(pair).mint(to);

        if (msg.value > amountMON) {
            (bool success, ) = msg.sender.call{value: msg.value - amountMON}("");
            require(success, "AmmRouter: refund failed");
        }
    }

    function removeLiquidity(
        address tokenA,
        address tokenB,
        uint256 liquidity,
        uint256 amountAMin,
        uint256 amountBMin,
        address to,
        uint256 deadline
    ) public ensure(deadline) returns (uint256 amountA, uint256 amountB) {
        address pair = _pairFor(tokenA, tokenB);
        require(AmmPair(pair).transferFrom(msg.sender, pair, liquidity), "AmmRouter: LP transfer failed");
        (uint256 amount0, uint256 amount1) = AmmPair(pair).burn(to);
        (address token0, ) = _sortTokens(tokenA, tokenB);
        (amountA, amountB) = tokenA == token0 ? (amount0, amount1) : (amount1, amount0);
        require(amountA >= amountAMin, "AmmRouter: insufficient A amount");
        require(amountB >= amountBMin, "AmmRouter: insufficient B amount");
    }

    function removeLiquidityMON(
        address token,
        uint256 liquidity,
        uint256 amountTokenMin,
        uint256 amountMONMin,
        address to,
        uint256 deadline
    ) external ensure(deadline) returns (uint256 amountToken, uint256 amountMON) {
        (amountToken, amountMON) = removeLiquidity(
            token, address(wmon), liquidity, amountTokenMin, amountMONMin, address(this), deadline
        );
        IERC20(token).safeTransfer(to, amountToken);
        wmon.withdraw(amountMON);
        (bool success, ) = to.call{value: amountMON}("");
        require(success, "AmmRouter: MON transfer failed");
    }

    function _swap(address tokenIn, address tokenOut, uint256 amountOut, address to) internal {
        address pair = _pairFor(tokenIn, tokenOut);
        (address token0, ) = _sortTokens(tokenIn, tokenOut);
        (uint256 amount0Out, uint256 amount1Out) = tokenIn == token0 ? (uint256(0), amountOut) : (amountOut, uint256(0));
        AmmPair(pair).swap(amount0Out, amount1Out, to);
    }

    function swapExactTokensForTokens(
        uint256 amountIn,
        uint256 amountOutMin,
        address tokenIn,
        address tokenOut,
        address to,
        uint256 deadline
    ) external ensure(deadline) returns (uint256 amountOut) {
        address pair = _pairFor(tokenIn, tokenOut);
        (address token0, ) = _sortTokens(tokenIn, tokenOut);
        (uint112 r0, uint112 r1) = AmmPair(pair).getReserves();
        (uint256 reserveIn, uint256 reserveOut) = tokenIn == token0 ? (uint256(r0), uint256(r1)) : (uint256(r1), uint256(r0));

        amountOut = getAmountOut(amountIn, reserveIn, reserveOut);
        require(amountOut >= amountOutMin, "AmmRouter: insufficient output amount");

        IERC20(tokenIn).safeTransferFrom(msg.sender, pair, amountIn);
        _swap(tokenIn, tokenOut, amountOut, to);
    }

    function swapExactMONForTokens(
        uint256 amountOutMin,
        address tokenOut,
        address to,
        uint256 deadline
    ) external payable ensure(deadline) returns (uint256 amountOut) {
        address pair = _pairFor(address(wmon), tokenOut);
        (address token0, ) = _sortTokens(address(wmon), tokenOut);
        (uint112 r0, uint112 r1) = AmmPair(pair).getReserves();
        (uint256 reserveIn, uint256 reserveOut) = address(wmon) == token0 ? (uint256(r0), uint256(r1)) : (uint256(r1), uint256(r0));

        amountOut = getAmountOut(msg.value, reserveIn, reserveOut);
        require(amountOut >= amountOutMin, "AmmRouter: insufficient output amount");

        wmon.deposit{value: msg.value}();
        require(wmon.transfer(pair, msg.value), "AmmRouter: WMON transfer failed");
        _swap(address(wmon), tokenOut, amountOut, to);
    }

    function swapExactTokensForMON(
        uint256 amountIn,
        uint256 amountOutMin,
        address tokenIn,
        address to,
        uint256 deadline
    ) external ensure(deadline) returns (uint256 amountOut) {
        address pair = _pairFor(tokenIn, address(wmon));
        (address token0, ) = _sortTokens(tokenIn, address(wmon));
        (uint112 r0, uint112 r1) = AmmPair(pair).getReserves();
        (uint256 reserveIn, uint256 reserveOut) = tokenIn == token0 ? (uint256(r0), uint256(r1)) : (uint256(r1), uint256(r0));

        amountOut = getAmountOut(amountIn, reserveIn, reserveOut);
        require(amountOut >= amountOutMin, "AmmRouter: insufficient output amount");

        IERC20(tokenIn).safeTransferFrom(msg.sender, pair, amountIn);
        _swap(tokenIn, address(wmon), amountOut, address(this));
        wmon.withdraw(amountOut);
        (bool success, ) = to.call{value: amountOut}("");
        require(success, "AmmRouter: MON transfer failed");
    }
}
