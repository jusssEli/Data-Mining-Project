#include <iostream>
#include <string>
#include <map>
#include <set>
#include <vector>

#include <algorithm>
#include <fstream>
#include <iomanip>

using namespace std;

struct Rule
{
    set<string> precedent_;
    set<string> antecendent_;
    double confidence_;

    bool operator<(Rule const &other) const
    {
        if (precedent_ < other.precedent_)
            return true;
        if (other.precedent_ < precedent_)
            return false;
        if (precedent_ != other.precedent_)
        {
            return precedent_ < other.precedent_;
        }
        return antecendent_ < other.antecendent_;
    }
};

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

int countSupport(const set<string> &candidate, const vector<set<string>> &transactions)
double countSupport(const set<string> &candidate, const vector<set<string>> &transactions)
{
    int count = 0;
    double count = 0;
    for (auto &t : transactions)
        if (includes(t.begin(), t.end(), candidate.begin(), candidate.end()))
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
                                     int min_support_count,
                                     map<set<string>, int> &support_map)
                                     double min_support_count,
                                     map<set<string>, double> &support_map)
{
    vector<set<string>> Lk;
    for (auto &cand : Ck)
    {
        int count = countSupport(cand, transactions);
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

    string filename = "transactions.txt";
    double min_support_percent = 0;
    double min_support_percent = 0;    // set to zero because we want all the items/sets
    double min_rule_confidence = 0.50; // 50%

    vector<set<string>> transactions = readTransactions(filename);
    int num_transactions = transactions.size();
    int min_support_count = ceil((min_support_percent / 100.0) * num_transactions);

    map<set<string>, int> support_map;
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

    multiset<Rule> Rules;
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

    cout << "Recommender Rules:\n";
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
                double numerator = support_map[ruleUnion];
                double denominator = support_map[rule.precedent_];
                tempRule.confidence_ = numerator / denominator;
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

    // Print out Rules
    cout << "Recommender Rules:\n";
    for (auto &rule : ValidRules)
    {
        cout << "{ ";
        for (auto &item : rule.precedent_)
            cout << item << " ";
        cout << "} -> { ";
        for (auto &item : rule.antecendent_)
            cout << item << " ";
        cout << "}\n";
        cout << "} : Confidence: " << rule.confidence_ * 100 << "\n";
    }

    return 0;
