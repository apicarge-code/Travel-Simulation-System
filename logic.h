#ifndef LOGIC_H
#define LOGIC_H

#include <iostream>
#include <vector>
#include <iomanip>
#include <string>

enum destination { DomesticShort, DomesticLong, International };

// 将函数声明为 inline 以便在头文件中定义，防止多重定义错误
inline std::string desti(destination d) {
    switch (d) {
    case DomesticShort: return "国内短途";
    case DomesticLong:  return "国内长途";
    case International: return "国际游";
    default: return "未知目的地";
    }
}

class Luggage {
public:
    std::string name;
    Luggage() {}
    Luggage(std::string n) : name(n) {}
};

class Passenger {
public:
    std::string idname;
    Luggage* luggages;
    int luggagecount;
    destination dest;
    bool jointeam;
    int teamnum;

    Passenger() : luggages(nullptr), luggagecount(0), jointeam(false), teamnum(0) {}

    void init(char c, int lc, destination d, bool jt) {
        idname = "乘客";
        idname += c;
        luggagecount = lc;
        luggages = new Luggage[luggagecount];
        jointeam = jt;
        dest = d;
        teamnum = 0;
        for (int i = 0; i < luggagecount; i++) {
            luggages[i].name = std::string(1, c) + "0" + std::to_string(i + 1);
        }
    }

    int ticketprice() {
        switch (dest) {
        case DomesticShort: return 10;
        case DomesticLong:  return 100;
        case International: return 500;
        default: return 0;
        }
    }

    int lugfee() {
        if (dest == International && luggagecount > 2) {
            return (luggagecount - 2) * 100;
        }
        return 0;
    }

    void printinfo() {
        std::cout << "我是" << idname << "，";
        if (jointeam) std::cout << "报名旅行社，";
        else std::cout << "选择自由出行，";
        std::cout << desti(dest) << "，携带" << luggagecount << "件行李，行李编号是";
        for (int i = 0; i < luggagecount; i++) {
            std::cout << luggages[i].name << (i == luggagecount - 1 ? "，" : "、");
        }
        std::cout << "买票需付" << ticketprice() << "元";
        if (lugfee() > 0) {
            std::cout << "，行李托运费" << lugfee() << "元。" << std::endl;
        } else {
            std::cout << "。" << std::endl;
        }
    }

    ~Passenger() {
        if (luggages) delete[] luggages; 
    }
};

class Team {
public:
    Passenger* tp[6];
    destination dest;
    int passengercount;
    Team() : passengercount(0) {}
    Team(destination d, int count) : dest(d), passengercount(count) {}

    int totalticketprice() {
        int total = 0;
        for (int i = 0; i < passengercount; i++) {
            total += tp[i]->ticketprice();
        }
        return total;
    }

    int totallugfee() {
        int sum = 0;
        if (dest == International) {
            for (int i = 0; i < passengercount; i++) {
                sum += tp[i]->lugfee();
            }
        }
        return sum;
    }
};

class TravelAgency {
public:
    Team* teams[5];
    TravelAgency() {
        for (int i = 0; i < 5; i++) teams[i] = nullptr;
    }
    int totalp() {
        int n = 0;
        for (int i = 0; i < 5; i++) {
            if (teams[i]) n += teams[i]->passengercount;
        }
        return n;
    }
    void printinfoma() {
        std::cout << "\n--- 旅行社统计信息 ---" << std::endl;
        std::cout << "旅行社共计安排了5个旅行团，共计" << totalp() << "人报名。" << std::endl;
        std::cout << "目的地分别是：";
        for (int i = 0; i < 5; i++) std::cout << desti(teams[i]->dest) << (i == 4 ? "" : ", ");
        std::cout << std::endl;
    }
};

class TicketOffice {
public:
    int free_money[3];
    std::string free_names[3];
    int team_money[3];
    std::string team_names[3];

    TicketOffice() {
        for (int i = 0; i < 3; i++) {
            free_money[i] = 0;
            team_money[i] = 0;
            free_names[i] = "";
            team_names[i] = "";
        }
    }

    void buyfree(Passenger& p) {
        int fee = p.ticketprice() + p.lugfee();
        free_money[p.dest] += fee;
        free_names[p.dest] += p.idname + " ";
    }

    void buyteam(Team& t) {
        int fee = t.totalticketprice() + t.totallugfee();
        team_money[t.dest] += fee;
        for (int i = 0; i < t.passengercount; i++) {
            team_names[t.dest] += t.tp[i]->idname + " ";
        }
    }

    void printinfomation() {
        std::cout << "\n--- 售票处营业额统计 ---" << std::endl;
        std::string labels[3] = { "国内短途", "国内长途", "国际游" };
        for (int i = 0; i < 3; i++) {
            std::cout << labels[i] << "自由行：" << std::setw(5) << free_money[i] << "元；乘客姓名：" << (free_names[i] == "" ? "无" : free_names[i]) << std::endl;
            std::cout << labels[i] << "团购：    " << std::setw(5) << team_money[i] << "元；乘客姓名：" << (team_names[i] == "" ? "无" : team_names[i]) << std::endl;
        }
    }
};

#endif // LOGIC_H