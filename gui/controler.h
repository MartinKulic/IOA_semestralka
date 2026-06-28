//
// Created by ja on 31/05/2026.
//

#ifndef IOA_SEMESTRALKA_CONTROLER_H
#define IOA_SEMESTRALKA_CONTROLER_H
#include <cmath>
#include <mutex>
#include <thread>
#include <cstring>

#include "../fStar/fStar.hpp"
#include  "../fStar/NodeAllocator.hpp"
#include "../fStar/Alg/DistanceMatrix.hpp"
#include "../fStar/Alg/SimulatedAnnealing.hpp"
#include "../fStar/Loader.hpp"


using namespace Alg;

class Controler {
    private:
    fStar::FStar* star;
    NodeAllocator* loader;
    DistanceMatrix* distancaMatrix;
    vector<fStar::Node*> centers;

    //std::atomic<bool> algo_stop{false};
    std::atomic<bool> algo_running{false};
    std::atomic<bool> algo_stop_request{false};
    std::thread algo_thread;

    std::mutex algo_result_mtx;
    string algo_result = "Algorith not run yet";
    std::atomic<double> algo_current_temperature;
    std::mutex alg_big_result_mtx;
    string algo_big_result = "No algorithm result";

    std::function<void()> redraw_callback = nullptr;

private:
    void rebuildCenterContainer() {
        for (int i = 0; i < this->centers.size(); i++) {
            (this->centers[i])->belongs_to_p_group = i;
            this->centers[i]->is_center = true; // just in case - clearResult expects this
        }
    }

    void hardCentersRebuild() {
        this->centers.clear();
        auto nodeEnd = star->end_nodes();
        for (auto it = star->begin_nodes(); it != nodeEnd; ++it) {
            if ((*it)->is_center) {
                (*it)->belongs_to_p_group = this->centers.size();
                this->centers.push_back(*it);
            }
        }
    }

    float strToWeight(string sWeight) {
        float weight;
        try {
            weight = std::stof(sWeight);
        }catch (...) {
            throw std::invalid_argument( "Error while parsing weight " + sWeight );
        }

        if (weight < 0.0) {
            throw std::invalid_argument( "Weight needs to be positive number not " + sWeight );
        }

        return weight;
    }

    public:
    Controler(fStar::FStar* fstar, NodeAllocator* loader): star(fstar), loader(loader) {
        this->distancaMatrix = new DistanceMatrix(fstar);

        auto nodeEnd = star->end_nodes();
        hardCentersRebuild();
    };
    ~Controler() {
        if (algo_thread.joinable()) algo_thread.join();
        delete distancaMatrix;
    }

    string addNode(string name, string sx, string sy, bool is_center, fStar::Node** newNodeToRet) {
        if (this->algo_running.load()) {
            return "No Modification while algorith is running";
        }

        float x,y;
        try {
            x = std::stof(sx);
        }catch (...) {
            return "Error while parsing x " + sx;
        }
        try {
            y = std::stof(sy);
        }catch (...) {
            return "Error while parsing y " + sy;
        }


        try {
            fStar::Node* node = loader->MakeNode(name, x, y, is_center );
            star->addNode(node);
            *newNodeToRet = node;
        }catch (const std::exception& e) {
            return "Oparation failed\n" + std::string(e.what());
        }

        if (is_center) {
            (*newNodeToRet)->belongs_to_p_group = this->centers.size();
            this->centers.push_back(*newNodeToRet);
        }

        return "Node " + name + " added successfully to x " + sx + " y " + sy;
    };
    string deleteNode(int nodeToDelId) {
        if (this->algo_running.load()) {
            return "No Modification while algorith is running";
        }

        fStar::Node* nodeToDelete = star->getNode(nodeToDelId);
        if (!nodeToDelete) {
            return "Could not found node id " + nodeToDelId;
        }

        if (nodeToDelete->is_center) {
            this->centers.erase(this->centers.begin() + nodeToDelete->belongs_to_p_group);
            rebuildCenterContainer();
        }

        this->star->deleteNode(nodeToDelId);
        loader->DestroyNode(nodeToDelId);
        return "Node deleted";
    };
    string modifyNode(fStar::Node* nodeToMod, string newName, string snewX, string snewY, bool is_center) {
        if (this->algo_running.load()) {
            return "No Modification while algorith is running";
        }

        if (nodeToMod == nullptr) {           // <-- guard against spurious calls
            return "Why and more likely HOW TF is modifie node called";
        }

        float newX,newY;
        try {
             newX = std::stof(snewX);
        }catch (...) {
            return "Modifie Node - Error while parsing x" + snewX;
        }
        try {
            newY = std::stof(snewY);
        }catch (...) {
            return "Modifie Node - Error while parsing y" + snewY;
        }

        nodeToMod->name=newName;
        nodeToMod->x=newX;
        nodeToMod->y=newY;

        if (nodeToMod->is_center != is_center) {
            nodeToMod->is_center=is_center;

            if (is_center) {
                nodeToMod->belongs_to_p_group = this->centers.size();
                this->centers.push_back(nodeToMod);
            }
            else {
                this->centers.erase(this->centers.begin() + nodeToMod->belongs_to_p_group);
                rebuildCenterContainer();
                nodeToMod->belongs_to_p_group = fStar::Node::NO_GROUP;
            }
        }




        return "Sucsessfull updated node " + nodeToMod->name;
    };

    string addEdge(fStar::Node* from, fStar::Node* to, string sWeight) {
        if (this->algo_running.load()) {
            return "No Modification while algorith is running";
        }

        if (!to || !from) {
            return "Edge points are not defined";
        }

        float weight;
        try {
            weight = strToWeight(sWeight);
        }catch (const exception& e) {
            return e.what();
        }

        star->addEdge(from, to, weight);

        return "OK";
    };
    string deleteEdge(fStar::Edge edgeToDel) {
        if (this->algo_running.load()) {
            return "No Modification while algorith is running";
        }

        star->deleteEdge(edgeToDel.from->id, edgeToDel.to->id);
        return"Edge to " + edgeToDel.to->name + " deleted.";
    };
    string modifyEdge(fStar::Edge edge, string newWeoght) {
        if (this->algo_running.load()) {
            return "No Modification while algorith is running";
        }

        float newWeight;
        try {
            newWeight = strToWeight(newWeoght);
        }catch (const exception& e) {
            return e.what();
        }

        star->modifieEdge(edge.from->id, edge.to->id, newWeight);

        return "New Edge w " + newWeoght + " modified.";
    };

    float calculateEuclideanDistance(fStar::Node* from, fStar::Node* to) {
        if (!from || !to) {
            return -1.f;
        }
        return sqrt(pow(to->x - from->x,2)+pow(to->y - from->y, 2));
    }
    string calculateEuclideanDistance(fStar::Node* from, fStar::Node* to, string* dest) {
        if (!from || !to) {
            return "Edge point are not defined";
        }
        *dest = std::to_string(calculateEuclideanDistance(from, to));
        return "Value calculated";
    }

    string recalculateAllDistances() {
        if (this->algo_running.load()) {
            return "No Modification while algorith is running";
        }

        auto endEdgeIt = star->end_edges();
        for (auto edgeIt = star->begin_edges(); edgeIt != endEdgeIt; ++edgeIt) {
            fStar::Edge edge = *edgeIt;

            fStar::Node* from = edge.from;
            fStar::Node* to = edge.to;

            float newWeight = calculateEuclideanDistance(from, to);
            star->modifieEdge(from->id, to->id, newWeight, true);
        }

        return "Recalculate all distances";
    }

    string save(std::string path) {
        try {
            Loader::save(path, star, &this->algo_big_result, &this->algo_result);
        }catch (const exception& e) {
            return e.what();
        }

        return "Saved";
    }

    string load(std::string path,  bool ignoreId = false) {
        if (this->algo_running.load()) {
            return "Not permited while algorith is running";
        }
        try {
            Loader::load(path, star, loader, ignoreId, &this->algo_big_result, &this->algo_result);
        }catch (const exception& e) {
            return e.what();
        }

        hardCentersRebuild();

        delete(this->distancaMatrix);
        this->distancaMatrix = new DistanceMatrix(this->star);

        return "Loaded";
    }

    string rcalcucateDistanceMatrix() {
        if (this->algo_running.load()) {
            return "Not permited while algorith is running";
        }

        delete(this->distancaMatrix);
        this->distancaMatrix = new DistanceMatrix(this->star);
        return "Matrix recalculated";
    }

    void algorithm_task(int numOfCenter, float temperature, float cooling) {
        this->algo_current_temperature = temperature;

        float bestFoundSolution = std::numeric_limits<float>::infinity();
        try {
            SimulatedAnnealing simAnl = SimulatedAnnealing(this->star, this->D(), numOfCenter, &this->centers, this->algo_current_temperature, this->algo_stop_request, cooling);
            simAnl.Run(this->redraw_callback);
            bestFoundSolution = simAnl.GetSolution();
        } catch (const std::invalid_argument& e) {
            this->algo_result_mtx.lock();
            this->algo_result = e.what();
            this->algo_result_mtx.unlock();

            algo_running.store(false);
            return;
        }
        catch (const std::runtime_error& e) {
            this->algo_result_mtx.lock();
            this->algo_result = e.what();
            this->algo_result_mtx.unlock();

            algo_running.store(false);
            return;
        }

        this->algo_result_mtx.lock();
        this->algo_result = "Best Found Solution is: " + std::to_string(bestFoundSolution);
        this->algo_result_mtx.unlock();

        vector<vector<fStar::Node*>>* groupToCenter = new vector<vector<fStar::Node*>>();
        for (int centerid = 0; centerid < this->centers.size(); centerid++) {
            groupToCenter->push_back(vector<fStar::Node*>());
        }

        auto nodeEnd = star->end_nodes();
        for (auto nodeit = star->begin_nodes(); nodeit != nodeEnd; ++nodeit) {
            fStar::Node* node = *nodeit;
            groupToCenter->at(node->belongs_to_p_group).push_back(node);
        }

        this->alg_big_result_mtx.lock();
        this->algo_big_result = "";
        for (int i = 0; i < this->centers.size(); i++) {
            vector<fStar::Node*> nodesInGroup = groupToCenter->at(i);
            fStar::Node* centerNode = (centers.at(i));
            this->algo_big_result += "Center group " + std::to_string(i) + " - " + centerNode->name + "  id[" + std::to_string(centerNode->id) + "]\n";
            if (nodesInGroup.size() == 0) {
                this->algo_big_result += "    No nodes\n";
                continue;
            }
            for (fStar::Node* node : nodesInGroup) {
                this->algo_big_result += "    g" + std::to_string(node->belongs_to_p_group) + "   " + node->name + " - id[" + std::to_string(node->id) + "]\n";
            }
        }
        this->alg_big_result_mtx.unlock();

        delete groupToCenter;

        this->redraw_callback();

        algo_running.store(false);
    }

    string runAlgorithm(string sp, string stemperature, string scooling) {
        if (algo_running.load())
            return "Algorithm already running.";

        this->algo_stop_request.store(false);

        this->rcalcucateDistanceMatrix();

        int numOfCenter;
        float temperature;
        float cooling;
        try {
            numOfCenter = std::stoi(sp);
            temperature = std::stof(stemperature);
            cooling = std::stof(scooling);
        } catch (...) {
            return "Problem while parsing parameters";
        }

        algo_running.store(true);
        // Detach previous thread if finished (join is also fine).
        if (algo_thread.joinable())
            algo_thread.join();

        this->algo_result = "RUNNING";
        this->algo_big_result = "No Result";

        algo_thread = thread([=, this] {algorithm_task(numOfCenter, temperature, cooling); });

        return "Algorithm run";
    }

    string stopAlogithm() {
        this->algo_stop_request.store(true);

        if (!this->isAlgRunning()) {
            return "Alorim already finished";
        }

        if (algo_thread.joinable()) algo_thread.join();

        //this->algo_running.store(false);

        this->algo_result_mtx.lock();
        this->algo_result += "  Found so far";
        this->algo_result_mtx.unlock();

        return "Algorithm stoped, result is best found yet";
    }

    string clearResult() {if (this->algo_running.load()) {
            return "Not permited while algorith is running";
        }
        auto endNodeIt = star->end_nodes();
        for (auto nodeIt = star->begin_nodes(); nodeIt != endNodeIt; ++nodeIt) {
            fStar::Node* node = *nodeIt;

            // if (!node->is_center) {
            //     node->belongs_to_p_group = fStar::Node::NO_GROUP;
            // }
            node->belongs_to_p_group = fStar::Node::NO_GROUP;
        }

        this->rebuildCenterContainer();

        this->alg_big_result_mtx.lock();
        this->algo_big_result = "No Result";
        this->alg_big_result_mtx.unlock();

        return "Result cleared";
    }

    int center_id_to_group(fStar::Node* center) {
        if (!center->is_center) {
            return -1;
        }

        for (int i = 0; i < this->centers.size(); i++) {
            if (this->centers[i]->id == center->id) {
                return i;
            }
        }
        return -1;
    }

    void setRedrawCallback(std::function<void()> fun) {
        this->redraw_callback = std::move(fun);
    }

    fStar::FStar* getFStar() {
        return this->star;
    };

    string getAlgoResult() {
        string result;
        this->algo_result_mtx.lock();
        result = this->algo_result;
        this->algo_result_mtx.unlock();
        return result;
    }

    string getAlgFullResult() {
        string result;
        this->alg_big_result_mtx.lock();
        result = this->algo_big_result;
        this->alg_big_result_mtx.unlock();
        return result;
    }

    string getCurrentTemperature() {
         return std::to_string(this->algo_current_temperature);
    }

    bool isAlgRunning() {
        return this->algo_running.load();
    }

    DistanceMatrix* D() const {
        return this->distancaMatrix;
    }

};

#endif //IOA_SEMESTRALKA_CONTROLER_H
