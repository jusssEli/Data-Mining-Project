#include <iostream>
#include <string>
#include <map>
#include <set>
#include <vector>

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <sstream> 
#include <cmath>

using namespace std;

struct Rule
{
    set<string> precedent_;
    set<string> antecendent_;
    double confidence_;

    bool operator<(Rule const &other) const
    {
        if (precedent_ != other.precedent_)
        {
            return precedent_ < other.precedent_;
        }
        return antecendent_ < other.antecendent_;
    }
};

// reads only items (no names)
vector<set<string>> readTransactions(const string &filename)
{
    ifstream infile(filename);
    vector<set<string>> transactions;
    string line;

    while (getline(infile, line))
    {
        set<string> transaction;
        size_t start = 0, end;
        int NameCount = 0;
        // split the line by spaces
        while ((end = line.find(' ', start)) != string::npos)
        {
            string item = line.substr(start, end - start);
            if (!item.empty() && NameCount >= 2)
                transaction.insert(item);
            ++NameCount;
            start = end + 1;
        }

        // Add the last item (or the only item if no spaces)
        string lastItem = line.substr(start);
        if (!lastItem.empty())
            transaction.insert(lastItem);

        if (!transaction.empty())
            transactions.push_back(transaction);
    }

    return transactions;
}

// NEW: Reads names + items into a map
map<string, set<string>> readNamedTransactions(const string &filename)
{
    ifstream infile(filename);
    map<string, set<string>> namedTransactions;
    string line;

    while (getline(infile, line))
    {
        istringstream iss(line);
        string first, last, item;
        iss >> first >> last;
        string fullName = first + " " + last;

        set<string> items;
        while (iss >> item)
            items.insert(item);

        namedTransactions[fullName] = items;
    }
    return namedTransactions;
}

vector<set<string>> generateC1(const vector<set<string>> &transactions)
{
    set<string> all_items;
    for (auto &t : transactions)
        all_items.insert(t.begin(), t.end());

    vector<set<string>> C1;
    for (auto &item : all_items)
        C1.push_back({item});
    return C1;
}

double countSupport(const set<string> &candidate, const vector<set<string>> &transactions)
{
    double count = 0;
    for (auto &t : transactions)
    {
        bool foundFlag = true;
        for (auto &c : candidate)
            if (t.find(c) == t.end())
                foundFlag = false;
        if (foundFlag)
            count++;
    }
    return count;
}

vector<set<string>> filterCandidates(const vector<set<string>> &Ck,
                                     const vector<set<string>> &transactions,
                                     double min_support_count,
                                     map<set<string>, double> &support_map)
{
    vector<set<string>> Lk;
    for (auto &cand : Ck)
    {
        double count = countSupport(cand, transactions);
        support_map[cand] = count;
        if (count >= min_support_count)
            Lk.push_back(cand);
    }
    return Lk;
}

vector<set<string>> aprioriGen(const vector<set<string>> &Lprev)
{
    vector<set<string>> Ck;
    int k = Lprev[0].size() + 1;

    for (size_t i = 0; i < Lprev.size(); i++)
    {
        for (size_t j = i + 1; j < Lprev.size(); j++)
        {
            vector<string> v1(Lprev[i].begin(), Lprev[i].end());
            vector<string> v2(Lprev[j].begin(), Lprev[j].end());

            bool joinable = true;
            for (int x = 0; x < k - 2; x++)
            {
                if (v1[x] != v2[x])
                {
                    joinable = false;
                    break;
                }
            }
            if (joinable)
            {
                set<string> candidate = Lprev[i];
                candidate.insert(v2.back());
                Ck.push_back(candidate);
            }
        }
    }
    return Ck;
}

int main()
{

    // Apriori Algorithm Logic, produces itemsets with supports ------------------------------------------
    string filename = "transactions1.txt";
    double min_support_percent = 0;    // set to zero because we want all the items/sets
    double min_rule_confidence = 0.50; // 50%

    // Read both normal and name-indexed transactions
    vector<set<string>> transactions = readTransactions(filename);
    map<string, set<string>> namedTransactions = readNamedTransactions(filename);

    int num_transactions = transactions.size();
    int min_support_count = ceil((min_support_percent / 100.0) * num_transactions);

    map<set<string>, double> support_map;
    vector<set<string>> C1 = generateC1(transactions);
    vector<set<string>> Lk = filterCandidates(C1, transactions, min_support_count, support_map);

    vector<set<string>> all_frequent;
    all_frequent.insert(all_frequent.end(), Lk.begin(), Lk.end());

    while (!Lk.empty())
    {
        vector<set<string>> Ck = aprioriGen(Lk);
        Lk = filterCandidates(Ck, transactions, min_support_count, support_map);
        all_frequent.insert(all_frequent.end(), Lk.begin(), Lk.end());
    }
    //------------------------------------------------------------------------------------------------------

    // Generate all possible Rules from collection of itemsets
    // takes forever
    set<Rule> Rules;
    int maxCount = all_frequent[all_frequent.size() - 1].size();
    for (auto &itemset_prec : all_frequent)
    {
        for (auto &itemset_ant : all_frequent)
        {
            bool includes = false;
            for (auto &item : itemset_prec)
            {
                if (itemset_ant.count(item) > 0)
                {
                    includes = true;
                    break;
                }
            }
            if (itemset_prec.size() < maxCount && !includes)
            {
                int searchSize = maxCount - itemset_prec.size();
                if (itemset_ant.size() <= searchSize)
                {
                    Rule newRule;
                    newRule.precedent_ = itemset_prec;
                    newRule.antecendent_ = itemset_ant;
                    Rules.insert(newRule);
                }
            }
        }
    }

    // Set to hold "Valid" Rules, defined below
    set<Rule> ValidRules;

    // Select "Valid" Rules from set of all possible Rules
    // takes forever
    for (auto &rule : Rules)
    {
        set<string> ruleUnion; // union for confidence caluclation
        set_union(rule.precedent_.begin(), rule.precedent_.end(),
                  rule.antecendent_.begin(), rule.antecendent_.end(), inserter(ruleUnion, ruleUnion.begin()));
        // if the precendent/Union is actually in the support map, continue
        if (support_map.find(ruleUnion) != support_map.end() &&
            support_map.find(rule.precedent_) != support_map.end())
        {
            // if its supports are greater than 3 (arbitrart level to weed out small samples)
            if (support_map[rule.precedent_] > 3 && support_map[ruleUnion] > 3)
            {
                Rule tempRule;
                tempRule.precedent_ = rule.precedent_;
                tempRule.antecendent_ = rule.antecendent_;
                tempRule.confidence_ = support_map[ruleUnion] / support_map[rule.precedent_];
                ValidRules.insert(tempRule);
            }
        }
    }

    // Narrow down valid rules further:
    // only keep ones with high enough confidence && not 1:1 rules (remove redundant, not good rules)
    for (auto it = ValidRules.begin(); it != ValidRules.end();)
    {
        if (it->confidence_ < min_rule_confidence || (it->precedent_.size() == 1 && it->antecendent_.size() == 1))
        {
            it = ValidRules.erase(it);
        }
        else
        {
            ++it;
        }
    }

    // Output rules to a external file
    ofstream ruleFile("ApplyRules.txt");
    ruleFile << "Recommender Rules:\n";
    for (auto &rule : ValidRules)
    {
        ruleFile << "{ ";
        for (auto &item : rule.precedent_)
            ruleFile << item << " ";
        ruleFile << "} -> { ";
        for (auto &item : rule.antecendent_)
            ruleFile << item << " ";
        ruleFile << "} : Confidence: " << rule.confidence_ * 100 << "\n";
    }
    ruleFile.close();
    

    // Apply rules and get final solutions
    ofstream recFile("recommendations.txt");

    for (auto &nt : namedTransactions)
    {
        string name = nt.first;
        set<string> purchases = nt.second;
        set<string> recommendations;

        // Check each rule
        for (auto &rule : ValidRules)
        {
            bool match = true;
            for (auto &item : rule.precedent_)
            {
                if (purchases.find(item) == purchases.end())
                {
                    match = false;
                    break;
                }
            }
            if (match)
            {
                for (auto &item : rule.antecendent_)
                {
                    if (purchases.find(item) == purchases.end())
                    {
                        recommendations.insert(item);
                    }
                }
            }
        }

        // Only print users who get recommendations
        if (!recommendations.empty())
        {
            recFile << name << ": Purchased Items: ";
            for (auto &p : purchases)
                recFile << p << " ";

            recFile << " Recommended Items: ";
            for (auto &r : recommendations)
                recFile << r << " ";
            recFile << "\n";
        }
    }

    recFile.close();

    cout << "Rules saved to ApplyRules.txt\n";
    cout << "Recommendations saved to recommendations.txt\n";

    return 0;
}
