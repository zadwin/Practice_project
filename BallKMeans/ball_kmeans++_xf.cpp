#include <iostream>
#include <fstream>
#include <time.h>
#include <cstdlib>
#include <algorithm>
// 有这两个语句就不需要配置Digen环境了
#include <Eigen/Dense>
#include <unsupported/Eigen/FFT>
#include <vector>
#include <cfloat>
/*由于当我直接在.json文件中添加/usr/include/eigen3时， 编译器提示找不到Eigen/Dense 文件，因此我通过如下的方式设置了软链接：

cd /usr/include
sudo ln -sf eigen3/Eigen Eigen
sudo ln -sf eigen3/unsupported unsupported

编程的时候，就可以直接使用如下的方式来包含头文件：

#include <Eigen/Dense>
#include <unsupported/Eigen/FFT>

如果不想设置软链接，可以直接使用：

#include <eigen3/Eigen/Dense>
#include <eigen3/unsupported/Eigen/FFT>

由于Eigen库非常特殊，解压即可用，因为它只有头文件，因此，编程时连CMake都不需要了。
*/
using namespace std;
using namespace Eigen;

typedef float OurType;

typedef VectorXf VectorOur;

typedef MatrixXf MatrixOur;

typedef vector<vector<OurType> > ClusterDistVector;

typedef vector<vector<unsigned int> > ClusterIndexVector;

typedef Array<bool, 1, Dynamic> VectorXb;

typedef struct Neighbor
//Define the "neighbor" structure
{
    OurType distance;
    int index;
}a;//可能这里要有声明一个名称

typedef vector<Neighbor> sortedNeighbors;

// 计算SSE值
float computeSSE(MatrixOur &dataset, int k, MatrixOur centroids, VectorXi labels);
// 加载文件
MatrixOur load_data(const char* filename);

inline MatrixOur
update_centroids(MatrixOur& dataset, ClusterIndexVector& cluster_point_index, unsigned int k, unsigned int n,
                 VectorXb& flag,
                 unsigned int iteration_counter, MatrixOur& old_centroids);

inline void saveAsCsv(string filename, MatrixOur data);

inline void update_radius(MatrixOur& dataset, ClusterIndexVector& cluster_point_index, MatrixOur& new_centroids,
                          ClusterDistVector& temp_dis,
                          VectorOur& the_rs, VectorXb& flag, unsigned int iteration_counter, unsigned int& cal_dist_num,
                          unsigned int the_rs_size);

inline sortedNeighbors
get_sorted_neighbors_Ring(VectorOur& the_Rs, MatrixOur& centers_dis, unsigned int now_ball, unsigned int k,
                          vector<unsigned int>& now_center_index);

inline sortedNeighbors
get_sorted_neighbors_noRing(VectorOur& the_rs, MatrixOur& centers_dist, unsigned int now_ball, unsigned int k,
                            vector<unsigned int>& now_center_index);


inline void
cal_centers_dist(MatrixOur& new_centroids, unsigned int iteration_counter, unsigned int k, VectorOur& the_rs,
                 VectorOur& delta, MatrixOur& centers_dis);

inline MatrixOur cal_dist(MatrixOur& dataset, MatrixOur& centroids);

inline MatrixOur
cal_ring_dist_Ring(unsigned j, unsigned int data_num, unsigned int dataset_cols, MatrixOur& dataset,
                   MatrixOur& now_centers,
                   ClusterIndexVector& now_data_index);

inline MatrixOur
cal_ring_dist_noRing(unsigned int data_num, unsigned int dataset_cols, MatrixOur& dataset, MatrixOur& now_centers,
                     vector<unsigned int>& now_data_index);


void initialize(MatrixOur& dataset, MatrixOur& centroids, VectorXi& labels, ClusterIndexVector& cluster_point_index,
                ClusterIndexVector& clusters_neighbors_index,
                ClusterDistVector& temp_dis);


VectorXi ball_k_means_Ring(MatrixOur& dataset, MatrixOur& centroids, bool detail = false) {

    double start_time, end_time;


    bool judge = true;

    const unsigned int dataset_rows = dataset.rows();
    const unsigned int dataset_cols = dataset.cols();
    const unsigned int k = centroids.rows();

    ClusterIndexVector temp_cluster_point_index;
    ClusterIndexVector cluster_point_index;
    ClusterIndexVector clusters_neighbors_index;
    ClusterIndexVector now_data_index;
    ClusterDistVector temp_dis;

    MatrixOur new_centroids(k, dataset_cols);
    MatrixOur old_centroids = centroids;
    MatrixOur centers_dis(k, k);

    VectorXb flag(k);
    VectorXb old_flag(k);

    VectorXi labels(dataset_rows);
    VectorOur delta(k);

    vector<unsigned int> old_now_index;
    vector<OurType> distance_arr;

    VectorOur the_rs(k);

    unsigned int now_centers_rows;
    unsigned int iteration_counter;
    unsigned int num_of_neighbour;
    unsigned int neighbour_num;
    unsigned int cal_dist_num;
    unsigned int data_num;

    MatrixOur::Index minCol;
    new_centroids.setZero();
    iteration_counter = 0;
    num_of_neighbour = 0;
    cal_dist_num = 0;
    flag.setZero();

    //initialize cluster_point_index and temp_dis
    initialize(dataset, centroids, labels, cluster_point_index, clusters_neighbors_index, temp_dis);

    temp_cluster_point_index.assign(cluster_point_index.begin(), cluster_point_index.end());

    /*
    // 1、输出测试 验证cluster_point_index
    cout << "cluster_point_index: " << endl;
    for (int i  = 0;  i < k; i++)
    {
        int test_size = cluster_point_index[i].size();
        for(int j = 0; j < test_size; j++)
        {
            cout << cluster_point_index[i][j]+1 << "     ";
        }
        cout << endl;
    }

    // 2、输出测试 验证labels
    cout << "labels: " << endl;
    for (int i  = 0;  i < labels.size(); i++)
    {
        cout << labels[i] + 1<< endl;
    }
    exit(0);
    */

    start_time = clock();

    while (true) {
        old_flag = flag;
        //record cluster_point_index from the previous round
        cluster_point_index.assign(temp_cluster_point_index.begin(), temp_cluster_point_index.end());
        iteration_counter += 1;


        //update the matrix of centroids
        new_centroids = update_centroids(dataset, cluster_point_index, k, dataset_cols, flag, iteration_counter,
                                         old_centroids);

        /*
        // 1、输出测试 验证new_centroids
        cout << "new_centroids:    "  << iteration_counter << endl;
        cout << new_centroids << endl;

        // 2、输出测试 flag
        cout << "flag: " << endl;
        cout << flag << endl;
        // exit(0);
         */
        if (new_centroids != old_centroids) {
            //delta: distance between each center and the previous center
            delta = (((new_centroids - old_centroids).rowwise().squaredNorm())).array().sqrt();
            /*
            // 1、输出测试 验证new_centroids和old_centroids
            cout << "new_centroids: " << endl;
            cout << new_centroids << endl;
            cout << "old_centroids: " << endl;
            cout << old_centroids << endl;

            // 2、输出测试 delta
            cout << "delta: " << endl;
            cout << delta << endl;
            // exit(0);
            */

            old_centroids = new_centroids;

            //get the radius of each centroids
            update_radius(dataset, cluster_point_index, new_centroids, temp_dis, the_rs, flag, iteration_counter,
                          cal_dist_num,
                          k);
            /*
            // 1、输出测试 验证temp_dis和the_rs
            cout << "temp_dis: " << endl;
            for (int i = 0; i < k; i++)
            {
                int test_size = temp_dis[i].size();
                for (int j = 0; j < test_size; j++)
                {
                    cout << temp_dis[i][j]<< "     ";
                }
                cout << endl;
            }


            cout << "the_rs: " << endl;
            cout << the_rs << endl;

            // 2、输出测试 cal_dist_num
            cout << "cal_dist_num: " << endl;
            cout << cal_dist_num << endl;
            exit(0);
            */
            // Calculate distance between centers

            cal_centers_dist(new_centroids, iteration_counter, k, the_rs, delta, centers_dis);
            /*
            // 1、输出测试 验证new_centroids和centers_dis
            cout << "new_centrodis: " << endl;
            cout << new_centroids << endl;
            cout << "centers_dis: " << endl;
            cout << centers_dis << endl;
            exit(0);
            */

            flag.setZero();

            //returns the set of neighbors

            //nowball;
            unsigned int now_num = 0;
            for (unsigned int now_ball = 0; now_ball < k; now_ball++) {
                /*
                // 1、输出测试 验证cluster_neighbors_index
                cout << "cluster_neighbors_index: " << endl;
                for (int i = 0; i < k; i++)
                {
                    int test_size = clusters_neighbors_index[i].size();
                    for (int j = 0; j < test_size; j++)
                    {
                        cout << clusters_neighbors_index[i][j] << "     ";
                    }
                    cout << endl;
                }
                exit(0);
                */
                sortedNeighbors neighbors = get_sorted_neighbors_Ring(the_rs, centers_dis, now_ball, k,
                                                                      clusters_neighbors_index[now_ball]);
                /*
                // 1、输出测试 验证cluster_neighbors_index
                cout << "cluster_neighbors_index: " << endl;
                for (int i = 0; i < k; i++)
                {
                    int test_size = clusters_neighbors_index[i].size();
                    for (int j = 0; j < test_size; j++)
                    {
                        cout << clusters_neighbors_index[i][j] << "     ";
                    }
                    cout << endl;
                }
                cout << "neighbors: " << endl;
                for (int i = 0; i < neighbors.size(); i++)
                {
                    cout << neighbors[i].index << "   "  << neighbors[i].distance << endl;
                }
                exit(0);
                */

                now_num = temp_dis[now_ball].size();
                if (the_rs(now_ball) == 0) continue;

                //Get the coordinates of the neighbors and neighbors of the current ball
                old_now_index.clear();
                old_now_index.assign(clusters_neighbors_index[now_ball].begin(),
                                     clusters_neighbors_index[now_ball].end());
                clusters_neighbors_index[now_ball].clear();
                neighbour_num = neighbors.size();
                /*
                // 1、输出测试 验证old_now_index,clusters_neighbors_index,neighbours_num。
                cout << "old_now_index: " << endl;
                for (int i = 0; i < old_now_index.size(); i++)
                {
                    cout << old_now_index[i] << "   ";
                }
                cout << endl;
                cout << "cluster_neighbors_index: " << clusters_neighbors_index[now_ball].size()<< endl;
                cout << "neighbors_num: " << neighbour_num << endl;
                exit(0);
                */

                MatrixOur now_centers(neighbour_num, dataset_cols);

                for (unsigned int i = 0; i < neighbour_num; i++) {
                    clusters_neighbors_index[now_ball].push_back(neighbors[i].index);
                    now_centers.row(i) = new_centroids.row(neighbors[i].index);

                }
                num_of_neighbour += neighbour_num;
                now_centers_rows = now_centers.rows();

                /*
                // 1、输出测试 验证cluster_neighbors_index
                cout << "cluster_neighbors_index: " << endl;
                for (int i = 0; i < k; i++)
                {
                    int test_size = clusters_neighbors_index[i].size();
                    for (int j = 0; j < test_size; j++)
                    {
                        cout << clusters_neighbors_index[i][j] + 1 << "     ";
                    }
                    cout << endl;
                }
                cout << "now_centers: " << endl;
                cout << now_centers << endl;
                cout << "num_of_neighbour:" << num_of_neighbour <<  "   " << neighbour_num<< endl;
                cout << "now_centers_rows: " << now_centers_rows << endl;
                exit(0);
                 */

                judge = true;  // 它就是用来判断是否可以推出迭代的条件。

                if (clusters_neighbors_index[now_ball] != old_now_index)
                    judge = false;
                else {
                    for (int i = 0; i < clusters_neighbors_index[now_ball].size(); i++) {
                        if (old_flag(clusters_neighbors_index[now_ball][i]) != false) {
                            judge = false;
                            break;
                        }
                    }
                }

                if (judge) {
                    continue;
                }

                now_data_index.clear();
                distance_arr.clear();

                for (unsigned int j = 1; j < neighbour_num; j++) {
                    distance_arr.push_back(centers_dis(clusters_neighbors_index[now_ball][j], now_ball) / 2);
                    now_data_index.push_back(vector<unsigned int>());
                }

                for (unsigned int i = 0; i < now_num; i++) {
                    for (unsigned int j = 1; j < neighbour_num; j++) {
                        if (j == now_centers_rows - 1 && temp_dis[now_ball][i] > distance_arr[j - 1]) {
                            now_data_index[j - 1].push_back(cluster_point_index[now_ball][i]);
                            break;
                        }
                        if (j != now_centers_rows - 1 && temp_dis[now_ball][i] > distance_arr[j - 1] &&
                            temp_dis[now_ball][i] <= distance_arr[j]) {
                            now_data_index[j - 1].push_back(cluster_point_index[now_ball][i]);
                            break;
                        }
                    }
                }

                /*
                // 1、输出测试 验证temp_dis、distance_arr以及now_data_index.
                cout << "temp_dis: " << endl;
                for (int i = 0; i < k; i++)
                {
                    int test_size = temp_dis[i].size();
                    for (int j = 0; j < test_size; j++)
                    {
                        cout << temp_dis[i][j] << "     ";
                    }
                    cout << endl;
                }
                cout << "distance_arr: " <<  distance_arr.size() << endl;
                for (int i = 0; i < distance_arr.size(); i++)
                {
                    cout << distance_arr[i] << "    ";
                }
                cout << endl;
                cout << "now_data_index: " << endl;
                int nirows = now_data_index.size();
                cout << "nirows: " << nirows << endl;
                for (int i = 0; i < nirows; i++)
                {
                    int test_size = now_data_index[i].size();
                    for (int j = 0; j < test_size; j++)
                    {
                        cout << now_data_index[i][j] << "     ";
                    }
                    cout << endl;
                }

                exit(0);
                */

                judge = false;
                int lenth = old_now_index.size();
                //Divide area
                for (unsigned int j = 1; j < neighbour_num; j++) {
                    data_num = now_data_index[j - 1].size();

                    if (data_num == 0)
                        continue;

                    MatrixOur temp_distance = cal_ring_dist_Ring(j, data_num, dataset_cols, dataset, now_centers,
                                                                 now_data_index);
                    cal_dist_num += data_num * j;
                    /*
                    // 1、输出测试 验证data_in_areaa、cal_dist_num以及temo_distance.
                    cout << "temp_diatance: " << endl;
                    cout << temp_distance << endl;
                    cout << "cal_dist_num: " << cal_dist_num << endl;
                    exit(0);
                    */

                    unsigned int new_label;
                    for (unsigned int i = 0; i < data_num; i++) {
                        temp_distance.row(i).minCoeff(&minCol);
                        new_label = clusters_neighbors_index[now_ball][minCol];
                        if (labels[now_data_index[j - 1][i]] != new_label) {
                            flag(now_ball) = true;
                            flag(new_label) = true;

                            //Update localand global labels
                            vector<unsigned int>::iterator it = (temp_cluster_point_index[labels[now_data_index[j -
                                                                                                                1][i]]]).begin();
                            while ((it) != (temp_cluster_point_index[labels[now_data_index[j - 1][i]]]).end()) {
                                if (*it == now_data_index[j - 1][i]) {
                                    it = (temp_cluster_point_index[labels[now_data_index[j - 1][i]]]).erase(it);
                                    break;
                                }
                                else
                                    ++it;
                            }
                            temp_cluster_point_index[new_label].push_back(now_data_index[j - 1][i]);
                            labels[now_data_index[j - 1][i]] = new_label;
                            /*
                            // 1、输出测试 验证temp_cluster_point_index
                            cout << "temp_cluster_point_index: " << endl;
                            for (int i = 0; i < k; i++)
                            {
                                int test_size = temp_cluster_point_index[i].size();
                                for (int j = 0; j < test_size; j++)
                                {
                                    cout << temp_cluster_point_index[i][j] + 1 << "     ";
                                }
                                cout << endl;
                            }
                            cout << "labels" << endl;
                            for(int i = 0; i < labels.size(); i++)
                            {
                                cout << labels[i] << "     ";
                            }
                            cout << endl;
                            exit(0);
                            */

                        }
                    }
                }
            }
        }
        else
            break;
    }
    end_time = clock();
    cout << "新的中心：" << endl
         << new_centroids << endl;
    saveAsCsv("dataSet/质心.csv", new_centroids);
    // 计算平方误差SSE
    cout << "误差和为：" << computeSSE(dataset, k, new_centroids, labels) << endl;
    if (detail == true) {
        cout << "ball-k-means with dividing ring:" << endl;
        cout << "k                :                  ||" << k << endl;
        cout << "iterations       :                  ||" << iteration_counter << endl;
        cout << "The number of calculating distance: ||" << cal_dist_num << endl;
        cout << "The number of neighbors:            ||" << num_of_neighbour << endl;
        cout << "Time per round:                     ||"
             << (double)(end_time - start_time) / CLOCKS_PER_SEC * 1000 / iteration_counter << endl;

    }
    return labels;
}

VectorXi ball_k_means_noRing(MatrixOur& dataset, MatrixOur& centroids, bool detail = false) {

    double start_time, end_time;

    bool judge = true;

    const unsigned int dataset_rows = dataset.rows();
    const unsigned int dataset_cols = dataset.cols();
    const unsigned int k = centroids.rows();

    OurType stable_field_dist = 0;

    ClusterIndexVector temp_clusters_point_index;
    ClusterIndexVector clusters_point_index;
    ClusterIndexVector clusters_neighbors_index;

    ClusterDistVector point_center_dist;

    MatrixOur new_centroids(k, dataset_cols);
    MatrixOur old_centroids = centroids; // 通过这种方式赋值，old_centroids和centroids是一样的了。
    MatrixOur centers_dist(k, k);

    VectorXb flag(k); // 判断质心是否发生变化。
    VectorXb old_flag(k);

    VectorXi labels(dataset_rows);
    VectorOur delta(k);

    vector<unsigned int> now_data_index;
    vector<unsigned int> old_now_index;

    VectorOur the_rs(k);

    unsigned int now_centers_rows;
    unsigned int iteration_counter;
    unsigned int num_of_neighbour;
    unsigned int neighbour_num;
    unsigned int cal_dist_num;
    unsigned int data_num;

    //bool key = true;
    // 这里是声明一个矩阵的列索引
    MatrixOur::Index minCol;
    new_centroids.setZero();
    iteration_counter = 0;
    num_of_neighbour = 0;
    cal_dist_num = 0;
    flag.setZero();
    //initialize clusters_point_index and point_center_dist
    // 这里的步骤是对所有的数据进行簇的划分。比如labels等已经有了明确的分配关系。
    initialize(dataset, centroids, labels, clusters_point_index, clusters_neighbors_index, point_center_dist);

    // 输出测试
    // cout << clusters_neighbors_index.size() << "         " << clusters_neighbors_index[0].size() << endl;
    // for (int i = 0; i < clusters_point_index.size(); i++)
    // {
    //     cout << clusters_point_index[i][0] << "         "<< clusters_point_index[i][1]  << endl;
    // }



    temp_clusters_point_index.assign(clusters_point_index.begin(), clusters_point_index.end());  // 这里的temp也就是起着临时存储的，为了区分前后代。

    start_time = clock();

    while (true) {
        old_flag = flag;

        // record clusters_point_index from the previous round
        //  主要的用途就是上一代和当前代簇下标的记录。clusters_point_index才是参与计算的变量。
        clusters_point_index.assign(temp_clusters_point_index.begin(), temp_clusters_point_index.end());
        iteration_counter += 1;


        //update the matrix of centroids
        new_centroids = update_centroids(dataset, clusters_point_index, k, dataset_cols, flag, iteration_counter,
                                         old_centroids); // 主要想要得到的就是新质心。

        if (new_centroids != old_centroids) {

            //delta: distance between each center and the previous center
            delta = (((new_centroids - old_centroids).rowwise().squaredNorm())).array().sqrt();  // 计算前一代和后一代的质心距离差。

            old_centroids = new_centroids;

            //get the radius of each centroids  获得每个质心的半径，样本和质心的距离以及计算次数。主要关注与k和n相关的计算。
            update_radius(dataset, clusters_point_index, new_centroids, point_center_dist, the_rs, flag,
                          iteration_counter, cal_dist_num, k);
            // Calculate distance between centers，计算质心间的距离。为什么需要the_rs, delta这两个参数。因为这里是计算两个质心间的距离。
            // 但是下面这个距离该怎么利用还没有很清楚。主要是用于计算下一代的邻居关系上面。
            cal_centers_dist(new_centroids, iteration_counter, k, the_rs, delta, centers_dist);

            flag.setZero();

            //returns the set of neighbors，这里的目的就是为了获得邻居簇。

            //nowball;
            unsigned int now_num = 0;
            for (unsigned int now_ball = 0; now_ball < k; now_ball++) {
                // 获得排序的邻居簇。
                sortedNeighbors neighbors = get_sorted_neighbors_noRing(the_rs, centers_dist, now_ball, k,
                                                                        clusters_neighbors_index[now_ball]);

                now_num = point_center_dist[now_ball].size();
                if (the_rs(now_ball) == 0) continue;

                //Get the coordinates of the neighbors and neighbors of the current ball
                old_now_index.clear();
                old_now_index.assign(clusters_neighbors_index[now_ball].begin(),
                                     clusters_neighbors_index[now_ball].end());
                clusters_neighbors_index[now_ball].clear();
                neighbour_num = neighbors.size();

                MatrixOur now_centers(neighbour_num, dataset_cols);

                for (unsigned int i = 0; i < neighbour_num; i++) {
                    clusters_neighbors_index[now_ball].push_back(neighbors[i].index);
                    now_centers.row(i) = new_centroids.row(neighbors[i].index);

                }

                num_of_neighbour += neighbour_num;

                now_centers_rows = now_centers.rows();

                judge = true;

                if (clusters_neighbors_index[now_ball] != old_now_index)
                    judge = false;
                else {
                    for (int i = 0; i < clusters_neighbors_index[now_ball].size(); i++) {
                        if (old_flag(clusters_neighbors_index[now_ball][i]) != false) {
                            judge = false;
                            break;
                        }
                    }
                }

                if (judge) {
                    continue;
                }

                now_data_index.clear();

                stable_field_dist = the_rs(now_ball);

                for (unsigned int j = 1; j < neighbour_num; j++) { // 通过这个去找到真正的稳定域。
                    stable_field_dist = min(stable_field_dist,
                                            centers_dist(clusters_neighbors_index[now_ball][j], now_ball) / 2);
                }

                for (unsigned int i = 0; i < now_num; i++) {

                    if (point_center_dist[now_ball][i] > stable_field_dist) {
                        now_data_index.push_back(clusters_point_index[now_ball][i]); //得到处于稳定域
                    }

                }


                data_num = now_data_index.size();

                if (data_num == 0) {
                    continue;
                }

                MatrixOur temp_distance = cal_ring_dist_noRing(data_num, dataset_cols, dataset, now_centers,
                                                               now_data_index);

                cal_dist_num += data_num * now_centers.rows();


                unsigned int new_label;
                for (unsigned int i = 0; i < data_num; i++) {  // 环域中点的重新分配。
                    temp_distance.row(i).minCoeff(&minCol);  //clusters_neighbors_index(minCol)
                    new_label = clusters_neighbors_index[now_ball][minCol];
                    if (labels[now_data_index[i]] != new_label) {


                        flag(now_ball) = true;
                        flag(new_label) = true;

                        //Update local and global labels
                        vector<unsigned int>::iterator it = (temp_clusters_point_index[labels[now_data_index[i]]]).begin();
                        while ((it) != (temp_clusters_point_index[labels[now_data_index[i]]]).end()) {
                            if (*it == now_data_index[i]) {
                                it = (temp_clusters_point_index[labels[now_data_index[i]]]).erase(it);
                                break;
                            }
                            else {
                                ++it;
                            }
                        }
                        temp_clusters_point_index[new_label].push_back(now_data_index[i]);
                        labels[now_data_index[i]] = new_label;
                    }
                }

            }

        }
        else {
            break;
        }
    }
    end_time = clock();
    cout << "新的中心：" << endl << new_centroids << endl;
    saveAsCsv("dataSet/质心.csv", new_centroids);
    // 计算平方误差SSE
    cout << "误差和为：" << computeSSE(dataset, k, new_centroids, labels) << endl;

    if (detail == true) {
        cout << "ball-k-means without dividing ring:" << endl;
        cout << "k                :                  ||" << k << endl;
        cout << "iterations       :                  ||" << iteration_counter << endl;
        cout << "The number of calculating distance: ||" << cal_dist_num << endl;
        cout << "The number of neighbors:            ||" << num_of_neighbour << endl;
        cout << "Time per round:                     ||" << (double)(end_time - start_time) / CLOCKS_PER_SEC * 1000 / iteration_counter << endl;

    }
    return labels;
}

MatrixOur load_data(const char* filename) {
    /*

    *Summary: Read data through file path

    *Parameters:

    *     filename: file path.*    

    *Return : Dataset in eigen matrix format.

    */

    int x = 0, y = 0;  // x: rows  ，  y/x: cols
    ifstream inFile(filename, ios::in);
    string lineStr;
    while (getline(inFile, lineStr)) {
        stringstream ss(lineStr);
        string str;
        while (getline(ss, str, ','))
            y++;
        x++;
    }
    MatrixOur data(x, y / x);
    ifstream inFile2(filename, ios::in);
    string lineStr2;
    int i = 0;
    while (getline(inFile2, lineStr2)) {
        stringstream ss2(lineStr2);
        string str2;
        int j = 0;
        while (getline(ss2, str2, ',')) {
            data(i, j) = atof(const_cast<const char*>(str2.c_str()));
            j++;
        }
        i++;
    }
    return data;
}

inline MatrixOur
update_centroids(MatrixOur& dataset, ClusterIndexVector& cluster_point_index, unsigned int k, unsigned int n,
                 VectorXb& flag, unsigned int iteration_counter, MatrixOur& old_centroids) {
    /*

    *Summary: Update the center point of each cluster

    *Parameters:

    *     dataset: dataset in eigen matrix format.*   

    *     clusters_point_index: global position of each point in the cluster.* 

    *     k: number of center points.*  

    *     dataset_cols: data set dimensions*  

    *     flag: judgment label for whether each cluster has changed.*  

    *     iteration_counter: number of iterations.*  

    *     old_centroids: center matrix of previous round.*  

    *Return : updated center matrix.

    */

    unsigned int cluster_point_index_size = 0;
    unsigned int temp_num = 0;
    MatrixOur new_c(k, n); // 这里的n应该是指维度。
    VectorOur temp_array(n);
    for (unsigned int i = 0; i < k; i++) {
        temp_num = 0;
        temp_array.setZero();
        cluster_point_index_size = cluster_point_index[i].size();
        if (flag(i) != 0 || iteration_counter == 1) { // 因为这个函数会提前计算出质心是否改变，或者说是否有样本点的改变。
            for (unsigned int j = 0; j < cluster_point_index_size; j++) {
                temp_array += dataset.row(cluster_point_index[i][j]);
                temp_num++;
            }
            new_c.row(i) = (temp_num != 0)? (temp_array / temp_num) : temp_array;
        }
        else new_c.row(i) = old_centroids.row(i);
    }
    return new_c;
}

inline void update_radius(MatrixOur& dataset, ClusterIndexVector& cluster_point_index, MatrixOur& new_centroids,
                          ClusterDistVector& temp_dis, VectorOur& the_rs, VectorXb& flag,
                          unsigned int iteration_counter,
                          unsigned int& cal_dist_num, unsigned int the_rs_size) {

    /*

    *Summary: Update the radius of each cluster

    *Parameters:

    *     dataset: dataset in eigen matrix format.*   

    *     clusters_point_index: global position of each point in the cluster.* 

    *     new_centroids: updated center matrix.*  

    *     point_center_dist: distance from point in cluster to center*  

    *     the_rs: The radius of each cluster.*  

    *     flag: judgment label for whether each cluster has changed.*  

    *     iteration_counter: number of iterations.*  

    *     cal_dist_num: distance calculation times.* 

    *     the_rs_size: number of clusters.* 

    */

    OurType temp = 0;
    unsigned int cluster_point_index_size = 0; // 用来存储每个簇样本的个数。
    for (unsigned int i = 0; i < the_rs_size; i++) {
        cluster_point_index_size = cluster_point_index[i].size();
        if (flag(i) != 0 || iteration_counter == 1) {
            the_rs(i) = 0;
            temp_dis[i].clear();
            for (unsigned int j = 0; j < cluster_point_index_size; j++) {
                cal_dist_num++;
                temp = sqrt((new_centroids.row(i) - dataset.row(cluster_point_index[i][j])).squaredNorm());
                temp_dis[i].push_back(temp);
                if (the_rs(i) < temp) the_rs(i) = temp; // 计算样本和质心的距离，同时得出簇的半径。
            }
        }
    }
};

bool LessSort(Neighbor a, Neighbor b) {
    return (a.distance < b.distance);
}

//todo
inline sortedNeighbors
get_sorted_neighbors_Ring(VectorOur& the_Rs, MatrixOur& centers_dis, unsigned int now_ball, unsigned int k,
                          vector<unsigned int>& now_center_index) {

    /*

    *Summary: Get the sorted neighbors

    *Parameters:

    *     the_rs: the radius of each cluster.*   

    *     centers_dist: distance matrix between centers.* 

    *     now_ball: current ball label.*  

    *     k: number of center points*  

    *     now_center_index: nearest neighbor label of the current ball.*  

    */

    VectorXi flag = VectorXi::Zero(k);
    sortedNeighbors neighbors;

    Neighbor temp;
    temp.distance = 0;
    temp.index = now_ball;
    neighbors.push_back(temp);
    flag(now_ball) = 1;


    for (unsigned int j = 1; j < now_center_index.size(); j++) {
        if (centers_dis(now_ball, now_center_index[j]) == 0 ||
            2 * the_Rs(now_ball) - centers_dis(now_ball, now_center_index[j]) < 0) {
            flag(now_center_index[j]) = 1;
        }
        else {
            flag(now_center_index[j]) = 1;
            temp.distance = centers_dis(now_ball, now_center_index[j]);
            temp.index = now_center_index[j];
            neighbors.push_back(temp);
        }
    }


    for (unsigned int j = 0; j < k; j++) {
        if (flag(j) == 1) {
            continue;
        }
        if (centers_dis(now_ball, j) != 0 && 2 * the_Rs(now_ball) - centers_dis(now_ball, j) >= 0) {
            temp.distance = centers_dis(now_ball, j);
            temp.index = j;
            neighbors.push_back(temp);
        }

    }

    sort(neighbors.begin(), neighbors.end(), LessSort);
    return neighbors;
}

inline sortedNeighbors
get_sorted_neighbors_noRing(VectorOur& the_rs, MatrixOur& centers_dist, unsigned int now_ball, unsigned int k,
                            vector<unsigned int>& now_center_index) {
    /*

    *Summary: Get the sorted neighbors

    *Parameters:

    *     the_rs: the radius of each cluster.*   

    *     centers_dist: distance matrix between centers.* 

    *     now_ball: current ball label.*  

    *     k: number of center points*  

    *     now_center_index: nearest neighbor label of the current ball.*  

    */

    VectorXi flag = VectorXi::Zero(k); // 应该是用来记录是否为邻居关系的。
    sortedNeighbors neighbors;

    Neighbor temp;
    temp.distance = 0;
    temp.index = now_ball; // 因为是第一次排序，所以第一个邻居一定是自己，并且是最近的。
    neighbors.push_back(temp);
    flag(now_ball) = 1;


    for (unsigned int j = 1; j < now_center_index.size(); j++) {
        if (centers_dist(now_ball, now_center_index[j]) == 0 ||
            2 * the_rs(now_ball) - centers_dist(now_ball, now_center_index[j]) < 0) {
            flag(now_center_index[j]) = 1; // 不是邻居。
        }
        else {
            flag(now_center_index[j]) = 1;
            temp.distance = centers_dist(now_ball, now_center_index[j]);
            temp.index = now_center_index[j];
            neighbors.push_back(temp); // 是邻居。
        }
    }


    for (unsigned int j = 0; j < k; j++) {
        if (flag(j) == 1) {
            continue;
        }
        if (centers_dist(now_ball, j) != 0 && 2 * the_rs(now_ball) - centers_dist(now_ball, j) >= 0) {
            temp.distance = centers_dist(now_ball, j);
            temp.index = j;
            neighbors.push_back(temp);
        }

    }
    return neighbors;
}

inline void
cal_centers_dist(MatrixOur& new_centroids, unsigned int iteration_counter, unsigned int k, VectorOur& the_rs,
                 VectorOur& delta, MatrixOur& centers_dis) {

    /*

    *Summary: Calculate the distance matrix between center points

    *Parameters:

    *     new_centroids: current center matrix.*   

    *     iteration_counter: number of iterations.* 

    *     k: number of center points.*  

    *     the_rs: the radius of each cluster*  

    *     delta: distance between each center and the previous center.*  

    *     centers_dist: distance matrix between centers.*  

    */

    if (iteration_counter == 1) centers_dis = cal_dist(new_centroids, new_centroids).array().sqrt(); // 为了克服第一代的影响。
    else {
        for (unsigned int i = 0; i < k; i++) {
            for (unsigned int j = 0; j < k; j++) {
                if (centers_dis(i, j) >= 2 * the_rs(i) + delta(i) + delta(j))
                    centers_dis(i, j) = centers_dis(i, j) - delta(i) - delta(j);
                else {
                    centers_dis(i, j) = sqrt((new_centroids.row(i) - new_centroids.row(j)).squaredNorm());
                }
            }
        }
    }
}

inline MatrixOur cal_dist(MatrixOur& dataset, MatrixOur& centroids) {

    /*

    *Summary: Calculate distance matrix between dataset and center point

    *Parameters:

    *     dataset: dataset matrix.*   

    *     centroids: centroids matrix.* 

    *Return : distance matrix between dataset and center point.

    */

    return (((-2 * dataset * (centroids.transpose())).colwise() + dataset.rowwise().squaredNorm()).rowwise() +
            (centroids.rowwise().squaredNorm()).transpose());
}

inline MatrixOur
cal_ring_dist_Ring(unsigned j, unsigned int data_num, unsigned int dataset_cols, MatrixOur& dataset,
                   MatrixOur& now_centers,
                   ClusterIndexVector& now_data_index) {

    /*

    *Summary: Calculate the distance matrix from the point in the ring area to the corresponding nearest neighbor

    *Parameters:

    *     j: the label of the current ring.* 

    *     data_num: number of points in the ring area.*   

    *     dataset_cols: data set dimensions.* 

    *     dataset: dataset in eigen matrix format.* 

    *     now_centers: nearest ball center matrix corresponding to the current ball.* 

    *     now_data_index: labels for points in each ring.* 

    *Return : distance matrix from the point in the ring area to the corresponding nearest neighbor.

    */

    MatrixOur data_in_area(data_num, dataset_cols);

    for (unsigned int i = 0; i < data_num; i++) {
        data_in_area.row(i) = dataset.row(now_data_index[j - 1][i]);
    }
    Ref<MatrixOur> centers_to_cal(now_centers.topRows(j + 1));

    return (((-2 * data_in_area * (centers_to_cal.transpose())).colwise() +
             data_in_area.rowwise().squaredNorm()).rowwise() + (centers_to_cal.rowwise().squaredNorm()).transpose());
}

inline MatrixOur
cal_ring_dist_noRing(unsigned int data_num, unsigned int dataset_cols, MatrixOur& dataset, MatrixOur& now_centers,
                     vector<unsigned int>& now_data_index) {
    /*

    *Summary: Calculate the distance matrix from the point in the ring area to the corresponding nearest neighbor

    *Parameters:

    *     data_num: number of points in the ring area.*   

    *     dataset_cols: data set dimensions.* 

    *     dataset: dataset in eigen matrix format.* 

    *     now_centers: nearest ball center matrix corresponding to the current ball.* 

    *     now_data_index: labels for points in the ring.* 

    *Return : distance matrix from the point in the ring area to the corresponding nearest neighbor.

    */

    MatrixOur data_in_area(data_num, dataset_cols);

    for (unsigned int i = 0; i < data_num; i++) {
        data_in_area.row(i) = dataset.row(now_data_index[i]);
    }

    return (((-2 * data_in_area * (now_centers.transpose())).colwise() +
             data_in_area.rowwise().squaredNorm()).rowwise() + (now_centers.rowwise().squaredNorm()).transpose());
}

void initialize(MatrixOur& dataset, MatrixOur& centroids, VectorXi& labels, ClusterIndexVector& cluster_point_index,
                ClusterIndexVector& clusters_neighbors_index, ClusterDistVector& temp_dis) {

    /*

    *Summary: Initialize related variables

    *Parameters:

    *     dataset: dataset in eigen matrix format.*   

    *     centroids: centroids matrix.* 

    *     labels: the label of the cluster where each data is located.* 

    *     clusters_point_index: two-dimensional vector of data point labels within each cluster.* 

    *     clusters_neighbors_index: two-dimensional vector of neighbor cluster labels for each cluster.* 

    *     point_center_dist: distance from point in cluster to center.* 

    */

    MatrixOur::Index minCol;
    for (int i = 0; i < centroids.rows(); i++) {
        cluster_point_index.push_back(vector<unsigned int>());
        clusters_neighbors_index.push_back(vector<unsigned int>());
        temp_dis.push_back(vector<OurType>());
    }
    MatrixOur M = cal_dist(dataset, centroids);
    for (int i = 0; i < dataset.rows(); i++) {
        M.row(i).minCoeff(&minCol);
        labels(i) = minCol;
        cluster_point_index[minCol].push_back(i);
    }
}

inline MatrixOur initial_centroids(MatrixOur dataset, int k, int random_seed = -1) {
    // 这里是产生K-means++的代码
    int dataset_cols = dataset.cols();
    int dataset_rows = dataset.rows();
    vector<int> flag(dataset_rows, 0);

    MatrixOur centroids(k, dataset_cols);
    int initial_row = 0;
    if (random_seed == -1) {
        srand((unsigned)time(NULL));
        initial_row = rand() % dataset_rows;
    }
    else {
        initial_row = dataset_rows % random_seed;
        srand(random_seed);

    }
    centroids.row(0) = dataset.row(initial_row);
    flag[initial_row] = 1;

    vector<OurType> nearest(dataset_rows, 0);

    OurType t_dist = 0;

    for (int i = 0; i < k - 1; i++) {
        vector<OurType> p(dataset_rows, 0);

        for (int j = 0; j < dataset_rows; j++) {
            t_dist = sqrt((dataset.row(j) - centroids.row(i)).squaredNorm());
            if (i == 0 && flag[j] != 1) nearest[j] = t_dist;
            else if (t_dist < nearest[j]) nearest[j] = t_dist;

            if (j == 0) p[j] = nearest[j];
            else p[j] = p[j - 1] + nearest[j];
        }

        OurType rand_num = rand() % 1000000001;
        rand_num /= 1000000000;

        for (int j = 0; j < dataset_rows; j++) {
            p[j] = p[j] / p[dataset_rows - 1];
            if (rand_num < p[j]) {
                centroids.row(i + 1) = dataset.row(j);
                flag[j] = 1;
                nearest[j] = 0;
                break;
            }
        }
    }
    return centroids;
}

VectorXi ball_k_means(MatrixOur& dataset, int k, bool isRing = false, bool detail = false,
                      int random_seed = -1, const char* filename = "0") {
    MatrixOur centroids;
    filename = "dataset/centroids_ids2.csv";
    if (filename == "0") {
        centroids = initial_centroids(dataset, k, random_seed);
    }
    else {
        centroids = load_data(filename);
    }
    cout<<"初始化中心："<< endl <<centroids<<endl;
    VectorXi labels;
    if (isRing) {
        labels = ball_k_means_Ring(dataset, centroids, detail);
    }
    else {
        labels = ball_k_means_noRing(dataset, centroids, detail);
    }

    return labels;
}

void saveAsCsv(string filename, MatrixOur data)
{
    ofstream outFile;
    outFile.open(filename, ios::out);
    for (int i = 0; i < data.rows(); i++)
    {
        for (int j = 0; j < data.cols(); j++)
        {
            if (j != 0)
            {
                outFile << ",";
            }
            outFile << data(i, j);
        }
        outFile << endl;
    }
    outFile.close();
}

float computeSSE(MatrixOur &dataset, int k, MatrixOur centroids, VectorXi labels){
    float sumSSE = 0.0;
    float sum = 0.0;
    // cout << dataset.rows() << ":" << dataset.cols() << endl;
    for (int i = 0; i < dataset.rows(); i++)
    {
        for (int j = 0; j < dataset.cols(); j++)
        {
            sumSSE = sumSSE + pow( dataset(i,j) - centroids(labels(i, 0),j), 2 );
        }
    }
    return sumSSE;
}
/**
 *  ball k-means算法的源代码，这里是出自论文中，只是增加了对数据的保存等操作。
 */
int main(int argc, char *argv[])
{
    double start_time, end_time;
    start_time = clock();
    char fileName[30] = "dataset/ids2.csv";
    // MatrixOur dataset = load_data("dataSet/testSet.txt");
    MatrixOur dataset = load_data(fileName);
    VectorXi labels = ball_k_means(dataset, 4, true, true);
    // cout<<"输出："<<endl<<labels<<endl;  //矩阵可以直接整个输出。
    // 向文件中存入数据
    ofstream outFile;
    outFile.open("dataSet/标签.csv", ios::out);
    for (int i = 0; i < labels.rows(); i++)
    {
        for (int j = 0; j < labels.cols(); j++)
        {
            if (j != 0)
            {
                outFile << ",";
            }
            outFile << labels(i, j);
        }
        outFile << endl;
    }
    outFile.close();
    end_time = clock();
    cout << "最终执行时间为：" <<(double)(end_time - start_time) / CLOCKS_PER_SEC << endl;
}
