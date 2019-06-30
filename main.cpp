#include <vector>
#include <string>
#include <iostream>
#include <cstdio>
#include <algorithm>
#include <map>
#include <sstream>
#include <fstream>

#define INF 1073741823.0f
using namespace std;
// I get here
enum State {
    init_state = -1,
    wait_state,
    end_state
};
enum Dir {
    ErrCarDir = -1,
    D,
    L,
    R
};
enum RoadDir {
    ErrRoadDir = -1,
    SE,
    ES
};
int CarNum, RoadNum, CrossNum;
struct Car;
struct Road;
struct Cross;
vector<Car*> CarList;
vector<Road*> RoadList;
vector<Cross*> CrossList;
struct Car {
    int id;
    int from, to;
    int speed;
    int planTime;

    State state;
    int steps;

    Car(int i, int f, int t, int s, int p) : id(i), from(f), to(t), speed(s), planTime(p) {}
};


struct Road {
    int id;
    int length;
    int speed;
    int channel;
    int from, to;
    bool isDuplex;
    vector<vector<int>> carsOfRoadSE;
    vector<vector<int>> carsOfRoadES;

    Road(int i, int l, int s, int c, int f, int t, bool d)
        : id(i), length(l), speed(s), channel(c), from(f), to(t), isDuplex(d) {
        carsOfRoadSE.resize(length, vector<int>(channel, -1));
        carsOfRoadES.resize(length, vector<int>(channel, -1));
    }
};

map<int, int> RoadMap;
map<int, int> CarMap;
struct Cross {
    int id;
    vector<int> roads;

    vector<int> roadsId;

    Cross(int i, int r0, int r1, int r2, int r3): id(i) {
        roads.resize(4);
        roads[0] = r0;
        roads[1] = r1;
        roads[2] = r2;
        roads[3] = r3;
        roadsId.assign(roads.begin(), roads.end());
        for (int i = 0; i != 4; ++i) {
            if (roadsId[i] != -1) {
                roadsId[i] = RoadMap[roadsId[i]];
            }
        }

        sort(roadsId.begin(), roadsId.end());
    }
};

struct Path {
    //int indexOfCar;
    vector<int> PList;

    //Path(int i, vector<int>& p): indexOfCar(i), PList(p) {}
};

struct Solution
{
    vector<int> st_time;
    vector<Path> SList;
    int fitness;

    Solution() {
        st_time.resize(CarNum);
        SList.resize(CarNum);
        fitness = INF;
    }

};

Solution* Slt;
int numOfScheduledCars = 0;


void moveCarToNextRoad(int carId, int roadId, RoadDir rDir, int carPos, int carChannel, int nextRoadId, RoadDir nextRDir) {
    int v1 = min(CarList[carId]->speed, RoadList[roadId]->speed);
    int v2 = min(CarList[carId]->speed, RoadList[nextRoadId]->speed);
    int s1 = min(v1, carPos);
    int s2 = v2 - s1;
    for (int channel = 0; channel != RoadList[nextRoadId]->channel; ++channel) {
        for (int l = RoadList[nextRoadId]->length - 1; l >= 0; --l) {
            if (nextRDir == SE) {
                if (RoadList[nextRoadId]->carsOfRoadSE[l][channel] == -1) {
                    if (rDir == SE)
                        RoadList[roadId]->carsOfRoadSE[carPos][carChannel] = -1;
                    else // ES
                        RoadList[roadId]->carsOfRoadES[carPos][carChannel] = -1;
                    RoadList[nextRoadId]->carsOfRoadSE[l][channel] = carId;
                    ++numOfScheduledCars;
                    return;
                } else {
                    break;
                }

            } else { // ES
                if (RoadList[nextRoadId]->carsOfRoadES[l][channel] == -1) {
                    if (rDir == SE)
                        RoadList[roadId]->carsOfRoadSE[carPos][carChannel] = -1;
                    else // ES
                        RoadList[roadId]->carsOfRoadES[carPos][carChannel] = -1;
                    RoadList[nextRoadId]->carsOfRoadSE[l][channel] = carId;
                    ++numOfScheduledCars;
                    return;
                } else {
                    break;
                }
            }
        }
    }
}


char tmp;
char tmp_str[100];

void readCar(string& carPath) {
    FILE* fp = fopen(carPath.c_str(), "r");
    int id,from, to,speed, planTime;

    //read
    int index = 0;
    while ((tmp = fgetc(fp)) != EOF) {
        if (tmp == '#') {
            fgets(tmp_str, 1024, fp);
            continue;
        }
        else {
            fscanf(fp, "%d, %d, %d, %d, %d", &id, &from, &to, &speed, &planTime);
            fgets(tmp_str, 1024, fp);
            Car* car = new Car(id, from, to, speed, planTime);
            CarList.push_back(car);
            CarMap[id] = index++;
            ++CarNum;
        }
    }

    fclose(fp);
}

void readRoad(string& roadPath) {
    FILE* fp = fopen(roadPath.c_str(), "r");
    int id;
    int length;
    int speed;
    int channel;
    int from;
    int to;
    int bi;

    //read
    int index = 0;
    while ((tmp = fgetc(fp)) != EOF) {
        if (tmp == '#') {
            fgets(tmp_str, 1024, fp);
            continue;
        }
        else {
            fscanf(fp, "%d, %d, %d, %d, %d, %d, %d", &id, &length, &speed, &channel, &from, &to, &bi);
            fgets(tmp_str, 1024, fp);
            bool isDuplex = bi == 1 ? true : false;
            Road* road = new Road(id, length, speed, channel, from, to, isDuplex);
            RoadList.push_back(road);
            RoadMap[id] = index++;
            ++RoadNum;
        }
    }
    fclose(fp);
}

void readCross(string& crossPath) {
    FILE* fp = fopen(crossPath.c_str(), "r");
    int id;
    int r0, r1, r2, r3;

    //read
    while ((tmp = fgetc(fp)) != EOF) {
        if (tmp == '#') {
            fgets(tmp_str, 1024, fp);
            continue;
        }
        else {
            fscanf(fp, "%d, %d, %d, %d, %d", &id, &r0, &r1, &r2, &r3);
            fgets(tmp_str, 1024, fp);
            Cross* cross = new Cross(id, r0, r1, r2, r3);
            CrossList.push_back(cross);
            ++CrossNum;
        }
    }

    fclose(fp);
}

bool blocked(int roadId, int carId, int pos, int channel, int& blockedCarId, int& blockedCarPos) {
    int v = min(CarList[carId]->speed, RoadList[roadId]->speed);

    for (int l = pos - 1; l >= max(pos-v, 0); --l) {
        if (RoadList[roadId]->carsOfRoadSE[l][channel] != -1) {
            blockedCarId = RoadList[roadId]->carsOfRoadSE[l][channel];
            blockedCarPos = l;
            return true;
        }
    }
    blockedCarId = -1;
    blockedCarPos = -1;
    return false;
}
void driveAllCarJustOnRoadToEndState(int roadId) {
    Road* road = RoadList[roadId];
    for (int l = 0; l != road->length; ++l) {
        for (int ch = 0; ch != road->channel; ++ch) {
            int carId = road->carsOfRoadSE[l][ch];
            if (carId == -1) continue;
            Road* nextRoad = RoadList[Slt->SList[carId].PList[CarList[carId]->steps]];
            int v1 = min(CarList[carId]->speed, road->speed);
            int s1 = min(v1, l);
            int v2 = min(CarList[carId]->speed, nextRoad->speed);
            int s2 = min(v2, v2-s1);
            int blockedCarId;
            int bockedCarPos;
            if (!blocked(roadId, carId, l, ch, blockedCarId, bockedCarPos)) {
                if (s2 > 0) {
                    CarList[road->carsOfRoadSE[l][ch]]->state = wait_state;
                } else {
                    road->carsOfRoadSE[l][ch] = -1;
                    road->carsOfRoadSE[l-s1][ch] = carId;
                    CarList[carId]->state = end_state;
                }
            } else { //blocked
                if (CarList[blockedCarId]->state == wait_state) {
                    CarList[road->carsOfRoadSE[l][ch]]->state = wait_state;
                } else {
                    road->carsOfRoadSE[l][ch] = -1;
                    road->carsOfRoadSE[bockedCarPos+1][ch] = carId;
                    CarList[road->carsOfRoadSE[l][ch]]->state = end_state;
                }
            }
        }
    }

    for (int l = 0; l != road->length; ++l) {
        for (int ch = 0; ch != road->channel; ++ch) {
            int carId = road->carsOfRoadES[l][ch];
            if (carId == -1) continue;
            Road* nextRoad = RoadList[Slt->SList[carId].PList[CarList[carId]->steps]];
            int v1 = min(CarList[carId]->speed, road->speed);
            int s1 = min(v1, l);
            int v2 = min(CarList[carId]->speed, nextRoad->speed);
            int s2 = min(v2, v2-s1);
            int blockedCarId;
            int bockedCarPos;
            if (!blocked(roadId, carId, l, ch, blockedCarId, bockedCarPos)) {
                if (s2 > 0) {
                    CarList[road->carsOfRoadES[l][ch]]->state = wait_state;
                } else {
                    road->carsOfRoadES[l][ch] = -1;
                    road->carsOfRoadES[l-s1][ch] = carId;
                    CarList[carId]->state = end_state;
                }
            } else { //blocked
                if (CarList[blockedCarId]->state == wait_state) {
                    CarList[road->carsOfRoadES[l][ch]]->state = wait_state;
                } else {
                    road->carsOfRoadES[l][ch] = -1;
                    road->carsOfRoadES[bockedCarPos+1][ch] = carId;
                    CarList[road->carsOfRoadES[l][ch]]->state = end_state;
                }
            }
        }
    }
}

void driveAllCarJustOnRoadToEndState(int roadId, RoadDir rDir, int channel) {
    if (rDir == SE) {
        for (int l = 0; l != RoadList[roadId]->length; ++l) {
            int carId = RoadList[roadId]->carsOfRoadSE[l][channel];
            if (carId == -1 || CarList[carId]->state == end_state) continue; // 一个时间片内, 被调度过的车不再被调度
            Road* nextRoad = RoadList[Slt->SList[carId].PList[CarList[carId]->steps]];
            int v1 = min(CarList[carId]->speed, RoadList[roadId]->speed);
            int s1 = min(v1, l);
            int v2 = min(CarList[carId]->speed, nextRoad->speed);
            int s2 = min(v2, v2-s1);
            int blockedCarId;
            int bockedCarPos;
            if (!blocked(roadId, carId, l, channel, blockedCarId, bockedCarPos)) {
                if (s2 > 0) {
                    continue; // 仅仅处理该道路该车道上能在该车道内行驶后成为终止状态的车辆（对于调度后依然是等待状态的车辆不进行调度，且依然标记为等待状态
                } else {
                    RoadList[roadId]->carsOfRoadSE[l][channel] = -1;
                    RoadList[roadId]->carsOfRoadSE[l-s1][channel] = carId;
                    CarList[carId]->state = end_state;
                }
            } else { //blocked
                if (CarList[blockedCarId]->state == wait_state) {
                    continue; // CarList[road->carsOfRoadES[l][ch]]->state = wait_state;
                } else {
                    RoadList[roadId]->carsOfRoadSE[l][channel] = -1;
                    RoadList[roadId]->carsOfRoadSE[bockedCarPos+1][channel] = carId;
                    CarList[carId]->state = end_state;
                }
            }//else (v > s1) //因为车辆不能主动减速, 所以该车不可以走??? 还是走到channel的最前面???
        }
    } else { //ES
        for (int l = 0; l != RoadList[roadId]->length; ++l) {
            int carId = RoadList[roadId]->carsOfRoadES[l][channel];
            if (carId == -1 || CarList[carId]->state == end_state) continue; // 一个时间片内, 被调度过的车不再被调度
            Road* nextRoad = RoadList[Slt->SList[carId].PList[CarList[carId]->steps]];
            int v1 = min(CarList[carId]->speed, RoadList[roadId]->speed);
            int s1 = min(v1, l);
            int v2 = min(CarList[carId]->speed, nextRoad->speed);
            int s2 = min(v2, v2-s1);
            int blockedCarId;
            int bockedCarPos;
            if (!blocked(roadId, carId, l, channel, blockedCarId, bockedCarPos)) {
                if (s2 > 0) {
                    continue; // 仅仅处理该道路该车道上能在该车道内行驶后成为终止状态的车辆（对于调度后依然是等待状态的车辆不进行调度，且依然标记为等待状态
                } else {
                    RoadList[roadId]->carsOfRoadES[l][channel] = -1;
                    RoadList[roadId]->carsOfRoadES[l-s1][channel] = carId;
                    CarList[carId]->state = end_state;
                }
            } else { //blocked
                if (CarList[blockedCarId]->state == wait_state) {
                    continue; // CarList[road->carsOfRoadES[l][ch]]->state = wait_state;
                } else {
                    RoadList[roadId]->carsOfRoadES[l][channel] = -1;
                    RoadList[roadId]->carsOfRoadES[bockedCarPos+1][channel] = carId;
                    CarList[carId]->state = end_state;
                }
            }//else (v > s1) //因为车辆不能主动减速, 所以该车不可以走??? 还是走到channel的最前面???
        }
    }
}

RoadDir getDirection(int crossId, int roadId, bool outOfRoad) {
    // 必须在这里判断道路是否是双向
    if (RoadList[roadId]->isDuplex) {
        if (RoadList[roadId]->to == CrossList[crossId]->id)
            return outOfRoad ? SE : ES;
        else if (RoadList[roadId]->from == CrossList[crossId]->id)
            return outOfRoad ? ES : SE;
        else
            return ErrRoadDir;
    } else {
        if (RoadList[roadId]->to == CrossList[crossId]->id)
            return outOfRoad ? SE : ErrRoadDir;
        else if (RoadList[roadId]->from == CrossList[crossId]->id)
            return outOfRoad ? ErrRoadDir : SE;
        else
            return ErrRoadDir;
    }
}
/* 只有出路口的车辆才参与出路口的优先级调度 */
int getPriorityCarFromRoad(int roadId, RoadDir rDir, int& carPos, int& carChannel) {
    if (rDir == SE) {
        for (int l = 0; l != RoadList[roadId]->length; ++l) {
            for (int ch = 0; ch != RoadList[roadId]->channel; ++ch) {
                int carId = RoadList[roadId]->carsOfRoadSE[l][ch];
                if (carId != -1 && CarList[carId]->state == wait_state) {
                    carPos = l;
                    carChannel = ch;
                    return carId;
                }
            }
        }
    } else { //ES
        for (int l = 0; l != RoadList[roadId]->length; ++l) {
            for (int ch = 0; ch != RoadList[roadId]->channel; ++ch) {
                int carId = RoadList[roadId]->carsOfRoadES[l][ch];
                if (carId != -1 && CarList[carId]->state == wait_state) { // 当前位置有车并且是等待状态
                    carPos = l;
                    carChannel = ch;
                    return carId;
                }
            }
        }
    }
    carPos = -1;
    carChannel = -1;
    return -1;
}

Dir getDirectionOfCar(int crossId, int carId) {
    int curRoadId = Slt->SList[carId].PList[CarList[carId]->steps];
    int nextRoadId = Slt->SList[carId].PList[CarList[carId]->steps+1];
    int curRoadIdIndex, nextRoadIdIndex;
    for (int i = 0; i != 4; ++i) {
        if (CrossList[crossId]->roadsId[i] == curRoadId) {
            curRoadIdIndex = i;
        }
        if (CrossList[crossId]->roadsId[i] == nextRoadId) {
            nextRoadIdIndex = i;
        }
    }
    int diff = nextRoadIdIndex - curRoadIdIndex;
    if (diff == 1 || diff == -3) {
        return L;
    } else if (diff == -1 || diff == 3) {
        return R;
    } else if (diff == 2 || diff == -2) {
        return D;
    } else {
        return ErrCarDir;
    }
}

//int getRoadIdAtDir(int crossId, int RoadId, Dir dir) {
//    int curRoadIdIndex, atDirRoadIdIndex;
//    for (int i = 0; i != 4; ++i) {
//        if (CrossList[crossId]->roadsId[i] == RoadId) {
//            curRoadIdIndex = i;
//        }
//    }
//    if (dir == D) {
//        atDirRoadIdIndex = (curRoadIdIndex + 2) % 4;
//    } else if (dir == L) {
//        atDirRoadIdIndex = (curRoadIdIndex + 1) % 4;
//    } else if (dir == R) {
//        atDirRoadIdIndex = (curRoadIdIndex + 4 - 1) % 4;
//    }
//    return CrossList[crossId]->roadsId[atDirRoadIdIndex];
//}

bool canGetNewCarOnRoad(int roadId, RoadDir rDir) {
    for (int channel = 0; channel != RoadList[roadId]->channel; ++channel) {
        for (int l = RoadList[roadId]->length - 1; l >= 0; --l) {
            if (rDir == SE) {
                if (RoadList[roadId]->carsOfRoadSE[l][channel] == -1) {
                    return true;
                } else {
                    break;
                }
            } else { // ES
                if (RoadList[roadId]->carsOfRoadES[l][channel] == -1) {
                    return true;
                } else {
                    break;
                }
            }
        }
    }
    return false;
}

bool hasDCar(int crossId, int carId) {
    int nextRoadId = Slt->SList[carId].PList[CarList[carId]->steps];
    for (int r = 0; r != 4; ++r) {
        int otherRoadId = CrossList[crossId]->roadsId[r];
        if (otherRoadId == -1) continue;
        RoadDir otherRoadDir = getDirection(crossId, otherRoadId, true);
        if (otherRoadDir == ErrRoadDir) continue;
        int otherCarPos, otherCarChannel;
        int otherCarId = getPriorityCarFromRoad(otherRoadId, otherRoadDir, otherCarPos, otherCarChannel);
        if (otherCarId == -1) continue;
        Dir otherCarDir = getDirectionOfCar(crossId, otherCarId);
        int otherNextRoadId = Slt->SList[otherCarId].PList[CarList[otherCarId]->steps];
        if (nextRoadId == otherNextRoadId && otherCarDir == D) {
            return true;
        }
    }
    return false;
}

bool hasLCar(int crossId, int carId) {
    int nextRoadId = Slt->SList[carId].PList[CarList[carId]->steps];
    for (int r = 0; r != 4; ++r) {
        int otherRoadId = CrossList[crossId]->roadsId[r];
        if (otherRoadId == -1) continue;
        RoadDir otherRoadDir = getDirection(crossId, otherRoadId, true);
        if (otherRoadDir == ErrRoadDir) continue;
        int otherCarPos, otherCarChannel;
        int otherCarId = getPriorityCarFromRoad(otherRoadId, otherRoadDir, otherCarPos, otherCarChannel);
        if (otherCarId == -1) continue;
        Dir otherCarDir = getDirectionOfCar(crossId, otherCarId);
        int otherNextRoadId = Slt->SList[otherCarId].PList[CarList[otherCarId]->steps];
        if (nextRoadId == otherNextRoadId && otherCarDir == L) {
            return true;
        }
    }
    return false;
}

bool conflict(int crossId, int roadId, int carId) {
    Dir dir = getDirectionOfCar(crossId, carId);
    int nextRoadId = Slt->SList[carId].PList[CarList[carId]->steps+1];
    RoadDir rDir = getDirection(crossId, nextRoadId, false);
    if (dir == D) {
        if (canGetNewCarOnRoad(nextRoadId, rDir)) {
            return false;
        } else {
            return true;
        }
    } else if (dir == L) {
        if (hasDCar(crossId, carId) || !canGetNewCarOnRoad(nextRoadId, rDir)) { //有直行车辆还没走或者当前车辆将要上的道路没有空位置可上了, 左转车辆也不能走
            return true;
        } else {
            return false;
        }
    } else if (dir == R) {
        if (hasDCar(crossId, carId) || hasLCar(crossId, carId) || !canGetNewCarOnRoad(nextRoadId, rDir)){
            return true;
        } else {
            return false;
        }
    } else {
        cout << "无路可走" << endl;
        return true;
    }
}

int posOfLastCarOnRoadChannel(int roadId, RoadDir rDir, int channel) {
    int l;
    if (rDir == SE) {
        for (l = RoadList[roadId]->length - 1; l >= 0; --l) {
            if (RoadList[roadId]->carsOfRoadSE[l][channel] != -1)
                return l;
        }
        return -1; // l==-1, no car on road
    } else { // ES
        for (l = RoadList[roadId]->length - 1; l >= 0; --l) {
            if (RoadList[roadId]->carsOfRoadES[l][channel] != -1)
                return l;
        }
        return -1;
    }
}

bool driveCarInGarage(int curTime) {
    for (int carId = 0; carId != CarNum; ++carId) {
        if (Slt->st_time[carId] == curTime) {
            int first_roadId = Slt->SList[carId].PList[0];
            int v = min(CarList[carId]->speed, RoadList[first_roadId]->speed);
            RoadDir rDir = getDirection(CarList[carId]->from, first_roadId, false);
            if (rDir == SE) {
                int channel;
                for (channel = 0; channel != RoadList[first_roadId]->channel; ++channel) {
                    int s2 = min(v, RoadList[first_roadId]->length-1-posOfLastCarOnRoadChannel(first_roadId, rDir, channel));
                    if (s2 > 0) {
                        RoadList[first_roadId]->carsOfRoadSE[RoadList[first_roadId]->length-s2][channel] = carId;
                        break; //这辆车可以上路了, 去调度下一辆车库里的车吧
                        //不需要在这里标记车辆的状态
                    }
                }
                if (channel == RoadList[first_roadId]->channel) {
                    return false; // error: 该辆车到了实际发车时间, 但是无法上路, 出错.
                }
            } else {// ES
                int channel;
                for (channel = 0; channel != RoadList[first_roadId]->channel; ++channel) {
                    int s2 = min(v, RoadList[first_roadId]->length - 1 -
                                    posOfLastCarOnRoadChannel(first_roadId, rDir, channel));
                    if (s2 > 0) {
                        RoadList[first_roadId]->carsOfRoadES[RoadList[first_roadId]->length - s2][channel] = carId;
                        break; //这辆车可以上路了, 去调度下一辆车库里的车吧
                        //不需要在这里标记车辆的状态
                    }
                }
                if (channel == RoadList[first_roadId]->channel) {
                    return false; // error: 该辆车到了实际发车时间, 但是无法上路, 出错.
                }
            }
        }
    }
    return true;
}

int judge() { /* return time periods */
    cout << "begin to judge... " << endl;
    int time_periods = 0;
    int numOfCarsReachedDest = 0;
    for (; numOfCarsReachedDest != CarNum; ++time_periods/*按时间片处理*/) {
        cout << "time_periods: " << time_periods << endl;
        int lastNumOfScheduledCars = numOfScheduledCars;
        //updateAllRoadsData();
        /* step 1 */
        for (int roadId = 0; roadId != RoadNum; ++roadId) {
            /* 调整所有道路上的车辆, 让道路上所有车辆前进
             * 分别标记出来等待的车辆
             * 和终止状态的车辆 */
            driveAllCarJustOnRoadToEndState(roadId); //对所有车道进行调整
        }

        /* step 2 */
        /* 处理所有路口、道路中处于等待状态的车辆 */
        bool allCrossesDead = false;
        while (!allCrossesDead/* all car in road run into end state */) { // 循环调度
            allCrossesDead = true;
            /* dirveAllWaitCar() */
            for (int crossId = 0; crossId != CrossNum; ++crossId) {
                bool theCrossDead = false;
                while (!theCrossDead) {
                    theCrossDead = true;
                    for (int r = 0; r != 4; ++r) {
                        int roadId = CrossList[crossId]->roadsId[r];
                        if (roadId == -1) continue;
                        RoadDir rDir = getDirection(crossId, roadId, true);
                        if (rDir == ErrRoadDir) continue; // 当前道路没有可以通过路口的道路方向
                        while (true) {
                            // 路口的优先级调度
                            int carPos, carChannel;
                            int carId = getPriorityCarFromRoad(roadId, rDir, carPos, carChannel); //返回当前道路上处于第一优先级的处于等待状态的车辆
                            if (carId == -1) break; //当前道路没有要过路口的车辆
                            if (conflict(crossId, roadId, carId)) {
                                break; // 若当前道路最高优先级的要过路口的车辆和其他道路的车辆有冲突, 则去调度下一条道路
                            }

                            theCrossDead = false;
                            allCrossesDead = false;
                            int nextRoadId = Slt->SList[carId].PList[CarList[carId]->steps];
                            RoadDir nextRDir = getDirection(crossId, nextRoadId, false);
                            ++CarList[carId]->steps;
                            moveCarToNextRoad(carId, roadId, rDir, carPos, carChannel, nextRoadId, nextRDir); //更新Road中的二维数组, 并更新通过路口的这辆车的state
                            bool hasReachedDest = CarList[carId]->to == crossId;
                            if (hasReachedDest) {
                                ++numOfCarsReachedDest;
                            }

                            driveAllCarJustOnRoadToEndState(roadId, rDir, carChannel); // 更新这条channel上的其他车辆的state, 因为channel头部的车辆出路口了, 会对这条channel上的其他车辆产生影响
                        } //// 当前道路上没有车要过路口时, 其他不过路口的车辆没有被调度.
                    }
                }
            }
        }

        /* step 3 */
        /* 车库中的车辆上路行驶 */
        bool canDriveCarsOnRealTime = driveCarInGarage(time_periods); // 判断是否可以让车辆在实际发车时间到的时候上路
        if (!canDriveCarsOnRealTime)
            return INF;
        if (time_periods != 0 && time_periods != 1 && lastNumOfScheduledCars == numOfScheduledCars) { // 如果开始的几个时间片没有车上路...
            return INF; //dead loop
        }
    }
    cout << "end judge... " << endl;
    return time_periods;
}

void readStdAns(std::string& answerPath) {
    ifstream inFile;
    inFile.open(answerPath.c_str());
    int id, realTime;
    stringstream ss;
    string s;
    int road;

    //read
    while (getline(inFile, s)) {
        cout << "s" << s;
        if (s[0] == '#') {
            continue;
        }
        s.erase(std::remove(s.begin(), s.end(), ','), s.end());
        ss.clear();
        ss.str(s);
        ss >> tmp >> id >> realTime;
        while (ss >> road) {
            Slt->SList[CarMap[id]].PList.push_back(RoadMap[road]);
            Slt->st_time[CarMap[id]] = realTime;
        }
        cout << endl;
    }
    inFile.close();
}

int main(int argc, char** argv)
{
    std::cout << "Begin" << std::endl;

    if (argc < 5)
    {
        std::cout << "please input args: carPath, roadPath, crossPath, answerPath" << std::endl;
        exit(1);
    }

    std::string carPath(argv[1]);
    std::string roadPath(argv[2]);
    std::string crossPath(argv[3]);
    std::string answerPath(argv[4]);

    std::cout << "carPath is " << carPath << std::endl;
    std::cout << "roadPath is " << roadPath << std::endl;
    std::cout << "crossPath is " << crossPath << std::endl;
    std::cout << "answerPath is " << answerPath << std::endl;

    readCar(carPath);
    readRoad(roadPath);
    readCross(crossPath);
    // TODO:process
    Solution solution;
    Slt = &solution;
    readStdAns(answerPath);
    int time_periods = judge();
    cout << "time_periods" << time_periods << endl;
    // TODO:write output file

    return 0;
}
