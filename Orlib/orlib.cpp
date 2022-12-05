#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <random>
#include <chrono>

enum flags
{
    TIME=1,
    GENERATION=2,
    SHOWCONSOLEOUTPUT=4
};

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

    void clearResult()
    {
        for(std::vector<long> &v : res)
        {
            v.clear();
        }
    }

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

std::ostream& operator<< (std::ostream& stream,Solution& s)
{
    stream<< s.timeElapsed << std::endl;
    for(const std::vector<long> &v : s.res)
    {
        for(const long &l : v)
        {
            stream << l << " ";
        }
        stream << std::endl;
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

/*

*/
void initializePopulation(Solution &empty,TestData &data,std::vector<Solution> &solutions,int &populationSize, int &seed)
{
    
    for(int i=0;i<populationSize;i++)
    {
        solutions.push_back(empty);
        randomizeSolution(solutions[i],seed);
        seed++;
        solutions[i]=genSolution(data,solutions[i]);
    }
    std::sort(solutions.begin(),solutions.end(),[](Solution &sol1,Solution &sol2){return sol1.timeElapsed<sol2.timeElapsed;}); //s1>s2 Sortowanie malejąco s1<s2 Sortowanie rosnąco

}

Solution crossover(Solution &s1,Solution &s2,Solution result,int preferred)
{
    result.priorityQueue.clear();
    std::random_device rd;
    std::uniform_int_distribution<int> dist(1,s1.priorityQueue.size()-2);
    int crossoverPoint=0,i=0;
    if(preferred<=70)
    {
        crossoverPoint=dist(rd);
        while(i<crossoverPoint)
        {
            result.priorityQueue.push_back(s1.priorityQueue[i]);
            i++;
        }
        i=0;
        while (i<(int)s2.priorityQueue.size())
        {
            if(!isInVector(result.priorityQueue,s2.priorityQueue[i]))
            {
                result.priorityQueue.push_back(s2.priorityQueue[i]);
            }
            i++;
        }
    }
    else
    {
        
        crossoverPoint=dist(rd);
        while(i<crossoverPoint)
        {
            result.priorityQueue.push_back(s2.priorityQueue[i]);
            i++;
        }
        i=0;
        while (i<(int)s1.priorityQueue.size())
        {
            if(!isInVector(result.priorityQueue,s1.priorityQueue[i]))
            {
                result.priorityQueue.push_back(s1.priorityQueue[i]);
            }
            i++;
        }
    }
    return result;
}

/*
Wykonuje losową ilość mutacji (Mutacje polegające na zamianie ze sobą dwóch liczb w kolejce priorytetowej)
[W zakresie od 1 do maxMutations]
*/
void mutate(Solution &s,int maxMutations)
{
    std::random_device rd; //Generator liczb losowych z biblioteki random 
    std::uniform_int_distribution<int> dist(0,s.priorityQueue.size()-1); //Rozkład liczb całkowitych w zakresie od 0 do wielkości kolejki priorytetu - 1 (Bo indeksujemy od 0) potrzebny do losowania które elementy w kolejce zostaną zamienione
    std::uniform_int_distribution<int> mutationDist(1,maxMutations);
    int mutationCount = mutationDist(rd);
    for(int i=0;i<mutationCount;i++)
    {
        int a = dist(rd),b = dist(rd);
        while (a==b)
        {
            b=dist(rd);
        }
    std::swap(s.priorityQueue[a],s.priorityQueue[b]);
    }
}

void generateNextPopulation(Solution &empty,TestData &data,std::vector<Solution> &solutions,int &populationSize,int breedingPercentage)
{
    
    int bestSolutionsCount=populationSize*breedingPercentage/100;
    std::vector<Solution> bestSolutions;
    std::random_device rd;
    std::uniform_int_distribution<int> dist(0,bestSolutionsCount-1);
    std::uniform_int_distribution<int> percentage(0,100);

    for(int i=0;i<bestSolutionsCount;i++)
    {
        bestSolutions.push_back(solutions[i]);
    }
    solutions.clear();

    int solutionToGenerate=populationSize-bestSolutionsCount;

    for(int i=0;i<bestSolutionsCount;i++)
    {
        solutions.push_back(bestSolutions[i]);
    }
    for(int i=solutionToGenerate;i>0;i--)
    {
        Solution tmp = crossover(bestSolutions[dist(rd)],bestSolutions[dist(rd)],empty,percentage(rd));
        if(percentage(rd)<60)
        {
            mutate(tmp,5);
        }
        solutions.push_back(genSolution(data,tmp));
    }
    bestSolutions.clear();
    std::sort(solutions.begin(),solutions.end(),[](Solution &sol1,Solution &sol2){return sol1.timeElapsed<sol2.timeElapsed;}); //s1>s2 Sortowanie malejąco s1<s2 Sortowanie rosnąco


}

void printGeneration(std::vector<Solution> &solutions,int n,int generation)
{
    if(n>=(int)solutions.size())
    {
        std::cout<<"Can't print this much" << std::endl;
    }
    std::cout<<"Generation: " << generation << std::endl;
    for(int i=0;i<n;i++)
    {
        std::cout<<solutions[i]<<std::endl;
    }
}

/*
Funkcja do zajmowania sie flagami programu [i argumentami ogólnie]
"-T ile" ~Czas Trwania Programu (W minutach)
"-G ile" ~Ile generacji ma przejść algorytm genetyczny
"-P ile" ~Jak duża ma być populacja
"-C" ~Wypisywanie w konsoli najlpeszych rozwiązań w każdej generacji
"-B ile" ~Wybranie jak dużo najlepszych rozwiązań ma zostać wypisanych w każdej generacji
*/
void flagHendeling(int &flag,std::string &filePath,std::string &outFilePath,int &stopAt,int &populationSize,int &showTop,int argc,char** argv)
{
    filePath = argv[1];
    if(argc<3)
    {
        outFilePath = filePath.substr(0,filePath.find('.',0))+"res.txt";
        return;
    }
    int i=2;
    if(argv[i][0]!='-')
    {
        outFilePath = argv[2];
        i=3;
    }
    else
    {
        i=2;
        outFilePath = filePath.substr(0,filePath.find('.',0))+"res.txt";
    }
    while(i<argc)
    {
        switch(argv[i][1])
        {
            case 'G':
            {
                flag = flag & 0b1110;
                flag = flag | flags::GENERATION; //Pojedyńcza "|" to binary or czyli porówny zmienimy konkretny bit we fladze na 1
                stopAt = atoi(argv[i+1]);
                i+=2;
                break;
            }
            case 'T':
            {
                flag = flag & 0b1101;
                flag = flag | flags::TIME;
                stopAt = atoi(argv[i+1]);
                i+=2;
                break;
            }
            case 'C':
            {
                flag = flag | flags::SHOWCONSOLEOUTPUT;
                i++;
                break;
            }
            case 'P':
            {
                populationSize = atoi(argv[i+1]);
                i+=2;
                break;
            }
            case 'B':
            {
                showTop = atoi(argv[i+1]);
                i+=2;
                break;
            }
            default:
                std::cout << "Niepoprawny argument" << std::endl;
                exit(1);
        }
    }
}

#pragma endregion

int main(int argc, char** argv)
{
    int stopAt=3;
    int flag = flags::TIME;// | flags::SHOWCONSOLEOUTPUT;
    std::string filePath = "test.txt";
    std::string outFilePath = filePath.substr(0,filePath.find('.',0))+"res.txt";
    int populationSize=100;
    int showTop=1;
    if(argc>1)
    {
        flagHendeling(flag,filePath,outFilePath,stopAt,populationSize,showTop,argc,argv);
    }
    int seed=time(nullptr);
    int generation = 0;
    TestData data;
    Solution emptySol;
    std::vector<Solution> solutions;
    
    readFile(filePath,data,emptySol);
    auto start = std::chrono::high_resolution_clock::now();
    initializePopulation(emptySol,data,solutions,populationSize,seed);
    auto currentTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(currentTime-start);
    
    if(flag & flags::SHOWCONSOLEOUTPUT)
    {
        printGeneration(solutions, showTop, generation++);
        std::cout<< duration.count() << " seconds elapsed" << std::endl;
    }
    

    if(flag & flags::GENERATION)
    {
    for(int i=0;i<stopAt;i++)
        {
            generateNextPopulation(emptySol,data,solutions,populationSize,10);
            currentTime = std::chrono::high_resolution_clock::now();
            duration = std::chrono::duration_cast<std::chrono::seconds>(currentTime-start);
            if(flag & flags::SHOWCONSOLEOUTPUT)
            {
                printGeneration(solutions,showTop,generation++);
                std::cout<< duration.count() << " seconds elapsed" << std::endl;
            }

        }
    }
    else
    {
        while(duration.count()<60*stopAt)
        {
            generateNextPopulation(emptySol,data,solutions,populationSize,10);
            currentTime = std::chrono::high_resolution_clock::now();
            duration = std::chrono::duration_cast<std::chrono::seconds>(currentTime-start);
            if(flag & flags::SHOWCONSOLEOUTPUT)
            {
                printGeneration(solutions,showTop,generation++);
                std::cout<< duration.count() << " seconds elapsed" << std::endl;
            }
        }
        
    }
    
    solutionToFile(outFilePath,data,solutions[0]);



    return 0;
}