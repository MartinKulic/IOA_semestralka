//
// Created by martin on 20. 6. 2026.
//

#include "SimulatedAnnealing.hpp"

#include <chrono>
#include <cmath>
#include <functional>
#include <list>

namespace Alg {
    void SimulatedAnnealing::MakeInitSolution() {
        // int i = 0;
        // for (; i < p; i++) {
        //     this->bestSolution[i] = centerCandidates->at(i)->id;
        //     this->included.push_back(centerCandidates->at(i)->id);
        // }
        // for (; i < centerCandidates->size(); i++) {
        //     this->notIncluded.push_back(centerCandidates->at(i)->id);
        // }

        std::vector<Node*> tempCandidates(centerCandidates->begin(), centerCandidates->end());
        for (int i = 0; i < p; i++) {
            int randInd = rand() % tempCandidates.size();
            Node* selectedNode = tempCandidates[randInd];

            this->bestSolution[i] = selectedNode->id;
            this->included.push_back(selectedNode->id);

            tempCandidates[randInd] = tempCandidates.back();
            tempCandidates.pop_back();
        }

        for (Node* node : tempCandidates) {
            this->notIncluded.push_back(node->id);
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

    void SimulatedAnnealing::AcceptNewBestSolution(float newFx) {
        int* solutionPt = this->bestSolution;
        for (int includedId: this->included) {
            *solutionPt = includedId;
            solutionPt++;
        }

        this->bestFx = newFx;
    }

    bool SimulatedAnnealing::Anneal(float newFx, float currentFx) {
        float probability = std::exp( -1 * (newFx - currentFx)/this->temperature );
        return probability >= (float)rand()/RAND_MAX;
    }

    float SimulatedAnnealing::CalculateFx() {
        float Fx = 0;

        auto nodeEnd = this->star->end_nodes();
        for (auto nodeIt = this->star->begin_nodes(); nodeIt != nodeEnd; ++nodeIt) {
            int closestCenter = getClosestIncludedCenterTo((*nodeIt)->id);
            if (closestCenter == -1) {
                throw std::runtime_error("Node " + (*nodeIt)->name + " - graf is not continuous");
            }

            Fx += (*D)[(*nodeIt)->id][closestCenter];

            // nech to blikaaaa
            (*nodeIt)->belongs_to_p_group = id_to_center_group[closestCenter];
        }

        return Fx;
    }


    void SimulatedAnnealing::Run(function<void()> redrawFun, int timeIntervalCallRedrawFun) {
        // init solution
        MakeInitSolution();

        this->bestFx = CalculateFx();

        if (this->notIncluded.size() == 0) {
            return;
        }

        auto last_notify = std::chrono::steady_clock::now();
        float currentFx = this->bestFx;

        while (temperature > 0.5 && !this->stop_requested.load()) {
            MakeNewSolution();

            float newFx = CalculateFx();
            if (currentFx < bestFx) {
                AcceptNewBestSolution(currentFx);
                currentFx = bestFx;
            } else {
                // random experiment
                if (Anneal(currentFx, currentFx)) {
                    //AcceptPrechod - cur solution in included
                    currentFx = newFx;
                } else {
                    RollbackSolution();
                }
            }
            //update temperature
            //this->temperature = this->temperature / (1 + (this->cooling * this->temperature) );
            this->temperature = this->temperature - this->cooling;
            auto now = std::chrono::steady_clock::now();

            if (redrawFun &&
                std::chrono::duration_cast<std::chrono::milliseconds>(now - last_notify).count() >= timeIntervalCallRedrawFun) {
                redrawFun();
                last_notify = now;
                }
        }
    }
} // Alg
