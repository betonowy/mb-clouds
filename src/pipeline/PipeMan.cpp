//
// Created by pekopeko on 10.10.2021.
//

#include "PipeMan.h"

#include <pipeline/assets/pip/final/SingleScatteringNearestCached.h>
#include <pipeline/assets/pip/final/SingleScatteringLinearCached.h>
#include <pipeline/assets/pip/final/SingleScatteringNearestBruteForce.h>
#include <pipeline/assets/pip/final/SingleScatteringLinearBruteForce.h>
#include <pipeline/assets/pip/final/MultipleScatteringNearestCached.h>
#include <pipeline/assets/pip/final/MultipleScatteringLinearCached.h>

PipeMan::PipeMan()
        : _pipelineMakers({{"Single Scattering Nearest Cached",             []() { return SingleScatteringNearestCached(); }},
                           {"Single Scattering Linear Cached",              []() { return SingleScatteringLinearCached(); }},
                           {"Multiple Scattering Nearest Partially Cached", []() { return MultipleScatteringNearestCached(); }},
                           {"Multiple Scattering Linear Partially Cached",  []() { return MultipleScatteringLinearCached(); }},
                           {"Single Scattering Nearest Brute force",        []() { return SingleScatteringNearestBruteForce(); }},
                           {"Single Scattering Linear Brute force",         []() { return SingleScatteringLinearBruteForce(); }},
                          }) {}
