#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <time.h>
#include <random>
#include <chrono>

///pragma region nie robi w sumie nic poza poza pozwoleniem na zwinięcie tekstu w edytorze [source do informacji https://learn.microsoft.com/pl-pl/cpp/preprocessor/region-endregion?view=msvc-170 ]

#pragma region structs

struct Task
{
    int mNumber;//Numer maszyny na której będzie wykonane zadanie
    int pTime;//Czas przetwarzania zadania
};

struct Job
{
    int id;
    int currentTask=0;
    std::vector<Task> tasks;

};

struct Machine
{
    int workingOnJob=-1;
    int timeLeft=0;
};

struct Solution
{
    long timeElapsed=0;
    std::vector<int> priorityQueue;
    std::vector<std::vector<long>> res;

    bool operator> (Solution &s)
    {
        if(timeElapsed>s.timeElapsed)
        {
            return true;
        }
        return false;
    }
    bool operator< (Solution &s)
    {
        if(timeElapsed<s.timeElapsed)
        {
            return true;
        }
        return false;
    }
};

struct TestData
{
    std::vector<Job> jobs;
    std::vector<Machine> machines;
    std::vector<int> jIndex;
    int numOfJobs;
    int numOfMachines;
};


//Tbh nie potrzebne użyte po to żeby sprawdzić czy dane wczytują sie poprawnie
std::ostream& operator<< (std::ostream& stream,Job& j)
{
    int i=0;
    for(Task t : j.tasks)
    {
        stream << "Task: " << i++ << " ( M " << t.mNumber << ", PT " << t.pTime << " ) "; 
    }
    return stream;
}


#pragma endregion

#pragma region functions

bool isInVector(std::vector<int> v, int num)
{
    for(int i : v)
    {
        if(i == num)
        {
            return true;
        }
    }
    return false;
}

void readFile(std::string filePath,TestData &data,Solution &sol)
{
    std::ifstream in;
    in.open(filePath);
    if(!in)
    {
        std::cout<<"Nie udało się otworzyć pliku " << filePath << std::endl;
        exit(1); 
    }
    in>>data.numOfJobs>>data.numOfMachines;
    for(int i=0;i<data.numOfJobs;i++)
    {
        Job tmpJob;
        tmpJob.id=i;
        for(int j=0;j<data.numOfMachines;j++)
        {
            Task tmpTask;
            in >> tmpTask.mNumber >> tmpTask.pTime;
            tmpJob.tasks.push_back(tmpTask);
        }
        data.jobs.push_back(tmpJob);
        sol.res.push_back(std::vector<long>{});
        sol.priorityQueue.push_back(i);
        data.jIndex.push_back(i);
    }
    in.close();
    for(int i=0;i<data.numOfMachines;i++)
    {
        Machine tmpMachine;
        data.machines.push_back(tmpMachine);
    }


}

void solutionToFile(std::string filePath,TestData &data, Solution &sol)
{
    std::ofstream out;
    out.open(filePath);
    out<< sol.timeElapsed << std::endl;
    for(int i = 0;i<data.numOfJobs;i++)
    {
        for(int j= 0; j<data.numOfMachines;j++)
        {
            out << sol.res[i][j] << " ";
        }
        out << std::endl;
    }
    out.close();
}

void randomizeSolution(Solution &sol,int &randomizationSeed)
{
    std::shuffle(sol.priorityQueue.begin(),sol.priorityQueue.end(),std::default_random_engine(randomizationSeed));
}

Solution genSolution(TestData data,Solution sol)
{
    std::vector<int> priorityQueue = sol.priorityQueue;
    while(!priorityQueue.empty())
    {
        int shortestTask = INT32_MAX;
        std::vector<int> neededMachines;
        for(Job j : data.jobs)
        {
            if(j.currentTask==data.numOfMachines)
            {
                continue;
            }
            if(!isInVector(neededMachines,j.tasks[j.currentTask].mNumber))
            {
                neededMachines.push_back(j.tasks[j.currentTask].mNumber);
            }
        }
        for(int m : neededMachines)
        {
            if(data.machines[m].timeLeft!=0)
            {
                if(shortestTask>data.machines[m].timeLeft)
                {
                    shortestTask=data.machines[m].timeLeft;
                }
                continue;
            }
            for(int j : priorityQueue)
            {
                if(!(data.jobs[j].tasks[data.jobs[j].currentTask].mNumber==m))
                {
                    continue;
                }
                if(shortestTask>data.jobs[j].tasks[data.jobs[j].currentTask].pTime)
                {
                    shortestTask=data.jobs[j].tasks[data.jobs[j].currentTask].pTime;
                }
                data.machines[m].timeLeft=data.jobs[j].tasks[data.jobs[j].currentTask].pTime;
                data.machines[m].workingOnJob=j;
                sol.res[j].push_back(sol.timeElapsed);
                break;
            }
        }
        for(int m : neededMachines)
        {
            data.machines[m].timeLeft-=shortestTask;
            if(data.machines[m].timeLeft==0)
            {
                data.jobs[data.machines[m].workingOnJob].currentTask++;
                if(data.jobs[data.machines[m].workingOnJob].currentTask==data.numOfMachines)
                {
                    priorityQueue.erase(std::find(priorityQueue.begin(),priorityQueue.end(),data.machines[m].workingOnJob));
                }
            }
        }
        neededMachines.clear();
        sol.timeElapsed+=shortestTask;
    }

    return sol;
}

Solution greedyMin(TestData data,Solution sol)
{
    while (!data.jIndex.empty())
    {
        int shortestTask = INT32_MAX;
        std::vector<int> neededMachines;
        for(int j : data.jIndex)
        {
            if(!isInVector(neededMachines,data.jobs[j].tasks[0].mNumber))
            {
                neededMachines.push_back(data.jobs[j].tasks[0].mNumber);
            }
        }

        for(int i : neededMachines)
        {
            if(data.machines[i].timeLeft!=0)
            {
                if(shortestTask>data.machines[i].timeLeft)
                {
                    shortestTask=data.machines[i].timeLeft;
                }
                continue;
            }
            
            int minTimeForMachine=INT32_MAX;
            int jobId;
            for(int j : data.jIndex)
            {
                if(!(data.jobs[j].tasks[0].mNumber==i))
                {
                    continue;
                }
                if(minTimeForMachine>data.jobs[j].tasks[0].pTime)
                {
                    minTimeForMachine=data.jobs[j].tasks[0].pTime;
                    jobId=data.jobs[j].id;
                }
            }
            data.machines[i].timeLeft=minTimeForMachine;
            data.machines[i].workingOnJob=jobId;
            sol.res[data.machines[i].workingOnJob].push_back(sol.timeElapsed);
            if(shortestTask>minTimeForMachine)
            {
                shortestTask=minTimeForMachine;
            }

        }
        for(int i : neededMachines)
        {
            data.machines[i].timeLeft-=shortestTask;
            if(data.machines[i].timeLeft==0)
            {
                data.jobs[data.machines[i].workingOnJob].tasks.erase(data.jobs[data.machines[i].workingOnJob].tasks.begin());
                if(data.jobs[data.machines[i].workingOnJob].tasks.empty())
                {
                    data.jIndex.erase(std::find(data.jIndex.begin(),data.jIndex.end(),data.machines[i].workingOnJob));
                }
            }
        }
        sol.timeElapsed+=shortestTask;
    }
    return sol;
}

Solution greedyMax(TestData data,Solution sol)
{
    while (!data.jIndex.empty())
    {
        int shortestTask = INT32_MAX;
        std::vector<int> neededMachines;
        for(int j : data.jIndex)
        {
            if(!isInVector(neededMachines,data.jobs[j].tasks[0].mNumber))
            {
                neededMachines.push_back(data.jobs[j].tasks[0].mNumber);
            }
        }

        for(int i : neededMachines)
        {
            if(data.machines[i].timeLeft!=0)
            {
                if(shortestTask>data.machines[i].timeLeft)
                {
                    shortestTask=data.machines[i].timeLeft;
                }
                continue;
            }
            
            int maxTimeForMachine=-1;
            int jobId;
            for(int j : data.jIndex)
            {
                if(!(data.jobs[j].tasks[0].mNumber==i))
                {
                    continue;
                }
                if(maxTimeForMachine<data.jobs[j].tasks[0].pTime)
                {
                    maxTimeForMachine=data.jobs[j].tasks[0].pTime;
                    jobId=j;
                }
            }
            data.machines[i].timeLeft=maxTimeForMachine;
            data.machines[i].workingOnJob=jobId;
            sol.res[data.machines[i].workingOnJob].push_back(sol.timeElapsed);
            if(shortestTask>maxTimeForMachine)
            {
                shortestTask=maxTimeForMachine;
            }

        }
        for(int i : neededMachines)
        {
            data.machines[i].timeLeft-=shortestTask;
            if(data.machines[i].timeLeft==0)
            {
                data.jobs[data.machines[i].workingOnJob].tasks.erase(data.jobs[data.machines[i].workingOnJob].tasks.begin());
                if(data.jobs[data.machines[i].workingOnJob].tasks.empty())
                {
                    data.jIndex.erase(std::find(data.jIndex.begin(),data.jIndex.end(),data.machines[i].workingOnJob));
                }
            }
        }
        sol.timeElapsed+=shortestTask;
    }
    return sol;
}

#pragma endregion

int main(int argc, char** argv)
{
    std::string filePath = "test.txt";
    std::string outFilePath = "res.txt";
    if(argc==2)
    {
        filePath = argv[1];
    }
    else if(argc==3)
    {
        //if(argv[0])
        filePath = argv[1];
        outFilePath = argv[2];
    }

    int seed=3;
    TestData data;
    Solution emptySol;
    Solution solMin;
    Solution solMax;
    Solution solTest;
    
    

    readFile(filePath,data,emptySol);
    solTest = emptySol;
    randomizeSolution(solTest,seed);
    solTest = genSolution(data,solTest);
    solutionToFile(outFilePath,data,solTest);
/*
    solMin = greedyMin(data,emptySol);
    solMax = greedyMax(data,emptySol);
    std::default_random_engine(1);
    if(solMax<solMin)
    {
        solutionToFile(outFilePath,data,solMax);
    }
    else
    {
        solutionToFile(outFilePath,data,solMin);
    }
*/

    return 0;
}