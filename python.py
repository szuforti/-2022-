# -*- coding: utf-8 -*- 
# 0320 version:4
# os - ubuntu 20.04
# python - 3.7.3
# numpy - 1.19.4
# 5% 优先分配法
# export PATH="~/anaconda3/bin:$PATH"   #将PATH添加即可
# source activate YourEnvs


import numpy as np
import configparser
import sys


class solver():
    def __init__(self, allow_data=None, step=2000, step_weaken=10, movement=0.0, max=50, Target=1000):
        self.allow_data = None
        self.step = step
        self.movement = movement
        self.max = max
        self.Target = Target
        self.step_weaken = step_weaken
        self.x = 0

    def once_time_slove(self, allow_data=None, client_need=None, node_up=None, node_down=None, solve_matrix=None):
        # 用于求解，
        last_Performance = float("inf")
        self.allow_data = allow_data
        big_tranform = int(client_need.min() * 0.05)
        # 解能不能成立
        slove_agree = False
        # 求初解
        if solve_matrix == None:
            # solve_matrix = self._slove_init(client_need=client_need)

            solve_matrix = self._slove_init_by_allow_site(client_need=client_need)

        # 用于冲量计算，记录上一次的矩阵
        gravitation_list_last = np.zeros(shape=node_up.shape)

        # 计算重力矩阵
        gravitation_list = self._calc_gravitation_list(solve_matrix=solve_matrix, node_up=node_up, node_down=node_down)
        col_solve_matrix = np.sum(solve_matrix, axis=0)
        if col_solve_matrix.max() - col_solve_matrix.min() < self.Target and gravitation_list.max() <= 1:
            # 排斥力是平均的，直接结束了
            return solve_matrix
        # 按行进行转移矩阵的计算，并开转移
        num = 0  # 循环计数器
        calc_num = 0  # 计算次数计数器
        step = self.step
        step_weaken = self.step_weaken
        while True:

            for i in range(solve_matrix.shape[0]):
                # print("calc", calc_num)
                calc_num = calc_num + 1
                # 计算传送矩阵
                if num != 1 or i != 0:
                    step = step - step_weaken
                    if step < 600:
                        step = 600
                    # print("strp", step)
                    # 添加冲量
                    gravitation_list = gravitation_list_last * self.movement + gravitation_list * (1 - self.movement)
                    gravitation_list_last = gravitation_list

                transport_list = self._calc_transport_list(solve_list=solve_matrix[i, :],
                                                           gravitation_list=gravitation_list,
                                                           allow_list=self.allow_data[i, :],
                                                           step=step,
                                                           solve_matrix=solve_matrix,
                                                           node_up=node_up)
                # 进行传送
                solve_matrix[i, :] = solve_matrix[i, :] + transport_list
                col_solve_matrix = np.sum(solve_matrix, axis=0)
                # print(col_solve_matrix)
                # print(np.std(col_solve_matrix))

                # 计算是否可以跳出
                # print(solve_matrix)
                gravitation_list = self._calc_gravitation_list(solve_matrix=solve_matrix, node_up=node_up,
                                                               node_down=node_down)
                col_solve_matrix = np.sum(solve_matrix, axis=0)

                Performance = col_solve_matrix.max() - col_solve_matrix.min()
                if Performance > last_Performance * 1.1 and gravitation_list.max() <= 1:
                    # print("到达最小值")
                    return solve_matrix
                last_Performance = Performance

                if col_solve_matrix.max() - col_solve_matrix.min() < self.Target and gravitation_list.max() <= 1:
                    slove_agree = True
                    # 排斥力相同
                    # print(num)
                    return solve_matrix
                if calc_num > 500:
                    sys.exit(0)
                if num > self.max and gravitation_list.max() <= 1:
                    # 如果超过规定的循环计数
                    # print("无解")
                    print("calc", calc_num)
                    return solve_matrix

                # print(solve_matrix)
                num = num + 1

    def _slove_init(self, client_need):
        # 计算初值，采用平均的方法计算
        solve_matrix = np.zeros(shape=self.allow_data.shape, dtype=np.int32)
        for i in range(client_need.size):
            if client_need[i] != 0:
                index_a = np.squeeze(np.argwhere(self.allow_data[i, :] == 1))
                if index_a.size == 0:
                    avg = 0
                    sys.exit(0)
                if index_a.size > 0:
                    avg = client_need[i] // index_a.size
                else_band = client_need[i] - (avg * (index_a.size - 1))
                try:
                    if index_a.size >= 2:
                        for ia in index_a[:-1]:
                            solve_matrix[i, ia] = avg
                        solve_matrix[i, index_a[-1]] = else_band
                    if index_a.size == 1:
                        solve_matrix[i, index_a] = else_band
                except:
                    sys.exit(0)
        return solve_matrix

    def _slove_init_by_allow_site(self, client_need):
        # 计算初值，尽量保持节点的占据相同,节点占越少，分的越多
        solve_matrix = np.zeros(shape=self.allow_data.shape, dtype=np.int32)
        allow_line_sum = client_need.size - np.sum(self.allow_data, axis=0) + 1
        for i in range(client_need.size):
            if client_need[i] != 0:
                index_a = np.squeeze(np.argwhere(self.allow_data[i, :] == 1))
                if index_a.size == 0:
                    pass
                if index_a.size == 1:
                    solve_matrix[i, index_a] = client_need[i]
                else:
                    denominator = np.sum(allow_line_sum[index_a])
                    # 求一份的大小
                    if denominator == 0:  # 全部都可以
                        avg = client_need[i] // self.allow_data[1, :].size
                        for ia in index_a[:-1]:
                            solve_matrix[i, ia] = avg
                        else_band = client_need[i] - np.sum(solve_matrix[i, index_a[:-1]])
                        solve_matrix[i, index_a[-1]] = else_band
                    else:
                        avg = client_need[i] // denominator
                        for ia in index_a[:-1]:
                            solve_matrix[i, ia] = avg * allow_line_sum[ia]
                        else_band = client_need[i] - np.sum(solve_matrix[i, index_a[:-1]])
                        solve_matrix[i, index_a[-1]] = else_band
        return solve_matrix

    def _calc_gravitation_list(self, solve_matrix=None, node_up=None, node_down=None):
        # 计算引力的列表，+为排斥，-为吸引 如果比他大就要分给他 ，如果相减比他少，就要要一点过来
        # gravitation_list = np.zeros(shape=node_up.shape)
        # 计算列和
        col_solve_matrix = np.sum(solve_matrix, axis=0)
        # 列和就是要的排斥力矩阵，将列和进行归一化，采用最大最小归一化

        gravitation_list = (col_solve_matrix - col_solve_matrix.min()) / (
                col_solve_matrix.max() - col_solve_matrix.min())

        for i in range(node_up.size):
            if col_solve_matrix[i] >= node_up[i] * 0.8:  # 如果比节点的上限大，这需要减少带宽，排斥掉,直接变成1
                gravitation_list[i] = 2
            if col_solve_matrix[i] <= node_down[i]:  # 如果比节点的下限小，这需要增加带宽，吸引，直接变成0
                gravitation_list[i] = 0

        return gravitation_list

    def Check_condition(self, solve_matrix=None, node_up=None, node_down=None):
        col_solve_matrix = np.sum(solve_matrix, axis=0)
        check_station = True
        for i in range(node_up.size):
            if col_solve_matrix[i] > node_up[i]:
                check_station = False
            if col_solve_matrix[i] < node_down[i]:
                check_station = False
        return check_station

    def _calc_transport_list(self, solve_list=None, gravitation_list=None, allow_list=None, step=600,
                             solve_matrix=None, node_up=None):
        # 计算引力的列表，+为排斥，-为吸引 如果比他大就要分给他 ，如果相减比他少，就要要一点过来
        # 计算搬迁的矩阵
        index_allow = np.argwhere(allow_list == 1)
        transport_list = np.zeros(shape=solve_list.shape, dtype=np.int32)
        max_moving_band = node_up - np.sum(solve_matrix, axis=0)
        for i, max_moving_band_value in enumerate(max_moving_band):
            if max_moving_band_value < 0:
                max_moving_band[i] = 0
        # 从每一个元素开始
        for j, ia in enumerate(index_allow):
            move_allow = np.delete(index_allow, j)
            for ma in move_allow:
                # 计算ia到ma的引力
                # 这里取反了，要注意
                gravitational = -1 * float(gravitation_list[ia] - gravitation_list[ma])
                # 计算需要搬运的带宽
                transport_band = round(gravitational * step)
                # 计算最大的搬入上限
                # -为排斥，+为吸引 如果比他大就要分给他 ，如果相减比他少，就要要一点过来
                # 计算搬迁的矩阵
                # 这是排斥的异常 排斥完了
                # transport_band = -2000 solve_list[ia]=3000 transport_list[ia]=-2000
                # 3000-2000-2000<0
                # less = 3000 +(-2000)
                # ia-1000
                # ma+1000
                if transport_band < 0 and solve_list[ia] + transport_band + transport_list[ia] <= 0:
                    # 如果ia搬走的太多，就不行，只能搬走还剩下的
                    less = int((solve_list[ia] + transport_list[ia]))
                    transport_list[ia] = transport_list[ia] - less
                    transport_list[ma] = transport_list[ma] + less
                # 这是吸收的异常  吸收玩了
                # transport_band = 2000 solve_list[ma]=3000 transport_list[ma]=-2000
                # 3000-2000-2000<0
                # less = 3000 -2000
                # ia+1000
                # ma-1000
                elif transport_band > 0 and solve_list[ma] - transport_band + transport_list[ma] <= 0:
                    # 如果ma搬走了太多，也要只能搬走剩下了的
                    less = int((solve_list[ma] + transport_list[ma]))
                    transport_list[ia] = transport_list[ia] + less
                    transport_list[ma] = transport_list[ma] - less
                # 这是排斥的异常  排斥多了
                # transport_band = -2000  transport_list[ia]=-2000 big_tranform=3000
                # -2000-2000>3000
                # less = 3000 -2000 = 1000
                # ia-1000
                # ma+1000
                elif transport_band < 0 and -1 * transport_band + transport_list[ma] >= max_moving_band[ma]:
                    # 也不能一进去太多，不然也会超上限
                    less = int((max_moving_band[ma] - transport_list[ma]))
                    transport_list[ia] = transport_list[ia] - less
                    transport_list[ma] = transport_list[ma] + less
                # 这是吸收的异常  吸收多了
                # transport_band = 2000  transport_list[ia]=2000 big_tranform=3000
                # 20002000>3000
                # less = 3000 -2000 = 1000
                # ia-1000
                # ma+1000
                elif transport_band > 0 and transport_band + transport_list[ia] >= max_moving_band[ia]:
                    less = int((max_moving_band[ia] - transport_list[ia]))
                    transport_list[ia] = transport_list[ia] + less
                    transport_list[ma] = transport_list[ma] - less
                # 正常情况
                # transport_band = 1000  transport_list[ia]=2000
                # ia  +1000
                # ma -1000
                else:
                    transport_list[ia] = transport_list[ia] + transport_band
                    transport_list[ma] = transport_list[ma] - transport_band
        # 输出前要保证不能变成负的
        return transport_list.copy()


class decider():

    def __init__(self, configPath="", DEMAND_PATH="", QOS_PATH="", SITE_BANDWIDTH_PATH="", max_ratio=0):
        QOS_CONSTRAINT, DEMAND_raw, QOS_raw, SB_raw = self._load_data(Config_Path=configPath, DEMAND_PATH=DEMAND_PATH,
                                                                      QOS_PATH=QOS_PATH,
                                                                      SITE_BANDWIDTH_PATH=SITE_BANDWIDTH_PATH)
        # 获得表头信息
        # 客户列表
        self.customers = DEMAND_raw[0, 1:]

        # 丢弃表头
        DEMAND_data = np.array(DEMAND_raw[1:, 1:], dtype=np.int32)
        QOS_data = np.array(QOS_raw[1:, 1:], dtype=np.int32)
        SB_data = np.array(SB_raw[1:, 1:], dtype=np.int32)

        # 这里加一个判断，如果节点不能使用的的话，直接在这里去掉，减少一点运算的成本。
        # step 1
        # 通过qos进行允许矩阵的计算：
        allow_data = np.int64(QOS_data < int(QOS_CONSTRAINT))
        # 求行和
        allow_data_line_sum = np.sum(allow_data, axis=1)
        # 得到行为0的index
        zero_line = np.squeeze(np.argwhere((allow_data_line_sum == 0)))
        if zero_line.size > 0:
            # 删除不要的数据
            self.allow_data = np.delete(allow_data, zero_line, axis=0)
            # 节点列表
            self.sites = np.delete(SB_raw[1:, 0], zero_line, axis=0)
            # qos列表
            self.QOS_data = np.delete(QOS_data, zero_line, axis=0)
            self.SB_data = np.delete(SB_data, zero_line, axis=0)
        else:
            self.allow_data = allow_data
            self.sites = SB_raw[1:, 0]
            self.QOS_data = QOS_data
            self.SB_data = SB_data
        # 时间信息
        self.times_num = DEMAND_data.shape[0]
        # 节点数量
        self.site_num = len(self.sites)
        # 客户数量
        self.customer_num = DEMAND_data.shape[1]
        # qos上限
        self.QOS_CONSTRAINT = QOS_CONSTRAINT
        # 客户需区数据
        self.DEMAND_data = DEMAND_data
        # 带宽列表

    def _load_data(self, Config_Path="", DEMAND_PATH="", QOS_PATH="", SITE_BANDWIDTH_PATH=""):
        # 导入数据
        conf = configparser.ConfigParser()
        # 读取.ini文件
        conf.read(Config_Path)
        # get()函数读取section里的参数值
        QOS_CONSTRAINT = conf.get("config", "qos_constraint")
        # print(QOS_CONSTRAINT)
        # QOS_CONSTRAINT = 1

        with open(DEMAND_PATH, encoding='utf-8') as f:
            DEMAND_raw = np.loadtxt(f, str, delimiter=",")
            # print(DEMAND_raw[:5])

        with open(QOS_PATH, encoding='utf-8') as f:
            QOS_raw = np.loadtxt(f, str, delimiter=",")
            # print(QOS_raw[:5])

        with open(SITE_BANDWIDTH_PATH, encoding='utf-8') as f:
            SB_raw = np.loadtxt(f, str, delimiter=",")
            # print(SB_raw[:5])

        return QOS_CONSTRAINT, DEMAND_raw, QOS_raw, SB_raw

    def _write_output(self, output_time_list, output_Path):

        with open(output_Path, 'w+', encoding="utf-8") as file_w:
            list_num = 0
            # list_max = len(output_time_list)
            for one_time_output_dict in output_time_list:
                for one_time_customer in one_time_output_dict.keys():
                    write_data = one_time_customer + ":"
                    for one_time_one_customer_one_site in one_time_output_dict[one_time_customer].keys():
                        if one_time_output_dict[one_time_customer][one_time_one_customer_one_site] != 0:
                            write_data = write_data + "<{},{}>,".format(one_time_one_customer_one_site,
                                                                        int(one_time_output_dict[
                                                                                one_time_customer][
                                                                                one_time_one_customer_one_site]))
                    if list_num == 0:
                        write_data = write_data.strip(",")

                    else:
                        write_data = "\n" + write_data.strip(",")

                    file_w.writelines(write_data)
                    list_num += 1

    def soler_priority(self, output_Path):
        Target = 1000000
        solv = solver(step=10000, step_weaken=50, movement=0.0, max=10, Target=Target)
        customer_output_dict = {}.fromkeys(self.customers)
        output_time_list = []  # 一行列表代表一个时间段
        node_up = np.squeeze(self.SB_data)
        node_down = np.zeros(shape=node_up.shape)
        for t in range(self.times_num):

            customer_need = self.DEMAND_data.copy()[t, :]
            for customer in customer_output_dict.keys():
                customer_output_dict[customer] = {}
            try:
                solve_matrix = solv.once_time_slove(allow_data=self.allow_data.T, client_need=customer_need,
                                                    node_down=node_down,
                                                    node_up=node_up - 10)
            except:
                sys.exit(0)
            # self._check_solve_matrix(allow_data=self.allow_data.T, solve_matrix=solve_matrix, node_up=node_up-1000,
            #                          custom_need=customer_need)

            customer_output_dict = self._solve_matrix2customer_output_dict(solve_matrix, customer_output_dict)

            output_time_list.append(customer_output_dict.copy())
        self._write_output(output_time_list, output_Path)

    def _solve_matrix2customer_output_dict(self, solve_matrix, customer_output_dict):
        for customer in customer_output_dict.keys():
            customer_output_dict[customer] = {}
        for customer_index in range(solve_matrix.shape[0]):
            for site_index in range(solve_matrix.shape[1]):
                customer_output_dict[self.customers[customer_index]][self.sites[site_index]] = solve_matrix[
                    customer_index, site_index]
        return customer_output_dict.copy()

    def _check_solve_matrix(self, allow_data=None, solve_matrix=None, node_up=None, custom_need=None):
        # 验证qos 不满足会输出不合法

        # 验证qos上限 不满足会正常退出
        col_solve_matrix = np.sum(solve_matrix, axis=0)
        print(col_solve_matrix)
        for i in range(col_solve_matrix.size):
            if col_solve_matrix[i] > self.SB_data[i]:
                sys.exit(1)
        # 不满足客户需求会异常退出
        line_solve_matrix = np.sum(solve_matrix, axis=1)
        for i in range(line_solve_matrix.size):
            if line_solve_matrix[i] != custom_need[i]:
                sys.exit(0)


def main():
    mode = 'linux'  # 文件路径
    if mode == 'linux':
        DEMAND_PATH = r'/data/demand.csv'
        QOS_PATH = r'/data/qos.csv'
        SITE_BANDWIDTH_PATH = r'/data/site_bandwidth.csv'
        configPath = r'/data/config.ini'
        output_path = r"/output/solution.txt"
    if mode == 'win':
        DEMAND_PATH = r'C:\Users\LHJ\Desktop\华为比赛\code2\data\demand.csv'
        QOS_PATH = r'C:\Users\LHJ\Desktop\华为比赛\code2\data\qos.csv'
        SITE_BANDWIDTH_PATH = r'C:\Users\LHJ\Desktop\华为比赛\code2\data\site_bandwidth.csv'
        configPath = r'C:\Users\LHJ\Desktop\华为比赛\code2\data\config.ini'
        output_path = r"C:\Users\LHJ\Desktop\华为比赛\code2\data\solution.txt"
    if mode == 'wintest':
        DEMAND_PATH = r'C:\Users\LHJ\Desktop\华为比赛\code2\CodeCraft2022-PressureGenerator-master\pressure_data\demand.csv'
        QOS_PATH = r'C:\Users\LHJ\Desktop\华为比赛\code2\CodeCraft2022-PressureGenerator-master\pressure_data\qos.csv'
        SITE_BANDWIDTH_PATH = r'C:\Users\LHJ\Desktop\华为比赛\code2\CodeCraft2022-PressureGenerator-master\pressure_data\site_bandwidth.csv'
        configPath = r'C:\Users\LHJ\Desktop\华为比赛\code2\CodeCraft2022-PressureGenerator-master\pressure_data\config.ini'
        output_path = r"C:\Users\LHJ\Desktop\华为比赛\code2\CodeCraft2022-PressureGenerator-master\pressure_data\solution.txt"

    x = decider(DEMAND_PATH=DEMAND_PATH, QOS_PATH=QOS_PATH, SITE_BANDWIDTH_PATH=SITE_BANDWIDTH_PATH,
                configPath=configPath)

    x.soler_priority(output_Path=output_path)


if __name__ == '__main__':
    # import time
    #
    # t1 = time.time()
    main()
    # t2 = time.time()
    # print(t2 - t1)
