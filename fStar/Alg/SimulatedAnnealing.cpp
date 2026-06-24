//
// Created by martin on 20. 6. 2026.
//

#include "SimulatedAnnealing.hpp"

#include <cmath>

namespace Alg {
    void SimulatedAnnealing::MakeInitSolution() {
        int i = 0;
        for (; i < p; i++) {
            this->bestSolution[i] = centerCandidates->at(i)->id;
            this->included.push_back(centerCandidates->at(i)->id);
        }
        for (; i < centerCandidates->size(); i++) {
            this->notIncluded.push_back(centerCandidates->at(i)->id);
        }
    }

    int SimulatedAnnealing::getClosestIncludedCenterTo(int nodeId) {
        DM_Row row = (*D)[nodeId];
        float minDisrance = std::numeric_limits<float>::infinity();
        int closestCenterId = -1;
        for (int includedId: this->included) {
            if (row[includedId] < minDisrance) {
                minDisrance = row[includedId];
                closestCenterId = includedId;
            }
        }
        return closestCenterId;
    }

    void SimulatedAnnealing::MakeNewSolution() {
        lastIndIncludedSelected = rand() % this->included.size();
        lastIndNotIncludedSelected = rand() % this->notIncluded.size();

        int helper = this->included.at(lastIndIncludedSelected);
        this->included.at(lastIndIncludedSelected) = this->notIncluded.at(lastIndNotIncludedSelected);
        this->notIncluded.at(lastIndNotIncludedSelected) = helper;
    }

    void SimulatedAnnealing::RollbackSolution() {
        int helper = this->notIncluded.at(lastIndNotIncludedSelected);
        this->notIncluded.at(lastIndNotIncludedSelected) = this->included.at(lastIndIncludedSelected);
        this->included.at(lastIndIncludedSelected) = helper;
    }

    void SimulatedAnnealing::AcceptSolution(float newFx) {
        int* solutionPt = this->bestSolution;
        for (int includedId: this->included) {
            *solutionPt = includedId;
            solutionPt++;
        }

        this->bestFx = newFx;
    }

    bool SimulatedAnnealing::Anneal(float newFx) {
        float probability = std::exp( -1 * (newFx - this->bestFx)/this->temperature );
        return probability <= rand();
    }

    float SimulatedAnnealing::CalculateFx() {
        float Fx = 0;

        auto nodeEnd = this->star->end_nodes();
        for (auto nodeIt = this->star->begin_nodes(); nodeIt != nodeEnd; ++nodeIt) {
            int closestCenter = getClosestIncludedCenterTo((*nodeIt)->id);
            if (closestCenter == -1) {
                throw std::runtime_error("Node " + (*nodeIt)->name + " has no center to belong to");
            }

            Fx += (*D)[(*nodeIt)->id][closestCenter];

            // nech to blikaaaa
            (*nodeIt)->belongs_to_p_group = id_to_center_group[closestCenter];
        }

        return Fx;
    }


    void SimulatedAnnealing::Run() {
        // init solution
        MakeInitSolution();

        float bestSoFar = CalculateFx();
        this->bestFx = bestSoFar;

        if (this->notIncluded.size() == 0) {
            return;
        }

        while (temperature > 0.5 && !this->stop_requested.load()) {
            MakeNewSolution();
            float currentFx = CalculateFx();
            if (currentFx < bestFx) {
                AcceptSolution(currentFx);
            } else {
                // random experiment
                if (Anneal(currentFx)) {
                    AcceptSolution(currentFx);
                } else {
                    RollbackSolution();
                }
            }
            //update temperature
            //this->temperature = this->temperature / (1 + (this->cooling * this->temperature) );
            this->temperature = this->temperature - this->cooling;
        }
    }
} // Alg
