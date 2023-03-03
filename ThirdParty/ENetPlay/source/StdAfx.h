/*
    @file StdAfx.h
    @date 06/03/2023
    @author Arves100
    @brief ENet Play stdafx
*/
#pragma once

// Strict windows definitions
#define STRICT 1

// Exclude rarely used Windows stuff
#define WIN32_LEAN_AND_MEAN 1

/// Remove min/max erorrs with enet or concurrentqueue
#define NOMINMAX 1

#include <stdint.h>
#include <enet.h>

#ifdef USE_DIRECTX
    // DirectX 8 SDK inclusions
    #include <dplay.h>
    #include <dplobby.h>
#else
    using DPID = uint32_t;
    static constexpr auto DPID_INVALID = 0xFFFFFFFFU;
    static constexpr auto DPID_SYSTEM = 0U;
#endif

// C++ inclusions
#include <thread>
#include <memory>
#include <mutex>

// unordered_dense
#include <unordered_dense.h>

// concurrent queue
#include <concurrentqueue.h>
