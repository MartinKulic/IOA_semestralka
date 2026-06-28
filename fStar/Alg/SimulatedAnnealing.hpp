//
// Created by martin on 20. 6. 2026.
//

#ifndef IOA_SEMESTRALKA_SUMULATEDANNEALING_HPP
#define IOA_SEMESTRALKA_SUMULATEDANNEALING_HPP
#include <atomic>
#include <ctime>
#include <functional>
#include <stdexcept>

#include "DistanceMatrix.hpp"
#include "../fStar.hpp"

namespace Alg {
    class SimulatedAnnealing {
        int p;
        const vector<fStar::Node*>* centerCandidates;
        vector<int> notIncluded;
        vector<int> included;
        map<int,int> id_to_center_group;
        int* bestSolution; //best solution
        float bestFx;
        atomic<double>& temperature;
        atomic<bool>& stop_requested;
        float cooling;
        fStar::FStar* star;
        DistanceMatrix* D;

        int lastIndIncludedSelected = 0;
        int lastIndNotIncludedSelected = 0;

        void MakeInitSolution();
        float CalculateFx();
        int getClosestIncludedCenterTo(int nodeId);
        void MakeNewSolution();
        void RollbackSolution();
        void AcceptNewBestSolution(float newFx);
        bool Anneal(float newFx);

        public:
        SimulatedAnnealing(fStar::FStar* star, Alg::DistanceMatrix* distanceMatrig, int numOfCenters, vector<fStar::Node*>* centerCandidates, atomic< double >& initTemperature, atomic<bool>& stop_flag, float cooling=10.0) : p(numOfCenters),
            centerCandidates(centerCandidates), temperature(initTemperature), cooling(cooling),
            star(star), D(distanceMatrig), stop_requested(stop_flag) {

            if ((*centerCandidates).size() < numOfCenters) {
                throw std::invalid_argument("Number of center candidates must be equal or greater than number of desired centers");
            }
            if (centerCandidates->size() < 1) {
                throw std::invalid_argument("There must be at least one center candidate");
            }
            bestSolution = new int[numOfCenters];

            for (int i = 0; i < centerCandidates->size(); i++) {
                id_to_center_group[ centerCandidates->at(i)->id ] = i;
            }

            srand(time(nullptr));
            //srand(10);
        };

        ~SimulatedAnnealing() {
            delete[] bestSolution;
        }


        void Run(function<void()> redrawFun = nullptr, int timeIntervalCallRedrawFun = 200);
        float GetSolution() {
            for (int i = 0; i < this->included.size(); i++) { // just in case
                this->included.at(i) = bestSolution[i];
            }
            CalculateFx();
            return bestFx;
        };

        void RequestStop() {
            this->stop_requested.store(true);
        }
        bool wasStopRequested() {
            return this->stop_requested.load();
        }
    };
} // Alg

#endif //IOA_SEMESTRALKA_SUMULATEDANNEALING_HPP
