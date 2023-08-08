import matplotlib.pyplot as plt

def read_process_info(file_path):
    processes = []
    with open(file_path, 'r') as file:
        for line in file:
            name, start, end = line.strip().split()
            processes.append({"name": name, "start": int(start), "end": int(end)})
    return processes

def draw_gantt_chart(processes, sch_, num):
    fig, gnt = plt.subplots(figsize=(15,8))

    gnt.set_xlabel('Time steps', fontsize=14)
    gnt.set_ylabel('Process', fontsize=14)

    max_time = max(process["end"] for process in processes)
    gnt.set_xlim(0, max_time)
    gnt.set_ylim(0, len(processes) + 1)

    # 設定 x 軸刻度標籤 (xticks)
    xticks = []
    for i, process in enumerate(processes):
        xticks.extend([process["start"], process["end"]])
    gnt.set_xticks(xticks)

    # 設定 y 軸刻度標籤 (yticks)
    yticks = [i + 1.0 for i in range(len(processes))]
    ytick_labels = [process["name"] for process in processes]
    gnt.set_yticks(yticks)
    gnt.set_yticklabels(ytick_labels)

    colors = plt.cm.tab10.colors

    # 繪製甘特圖並為每個行程指定不同的顏色
    for i, process in enumerate(processes):
        color = colors[i % len(colors)]
        gnt.broken_barh([(process["start"], process["end"] - process["start"])], (i + 0.85, 0.3), facecolors=(color))
    
    gnt.grid(True) 
    plt.setp(gnt.get_xticklabels(), rotation=45, ha="right",rotation_mode="anchor")
    #plt.tight_layout()  # 自動調整留白
    plt.title(str(sch_) + '_' +  str(num+1), fontsize=14)
    plt.savefig('figure/fig_'+ str(sch_) + '_' +  str(num+1) + '.png')

if __name__ == "__main__":
    SCH_ALG = ["FIFO", "PSJF", "RR", "SJF"]
    NUM_TEST = 5
    for sch_ in SCH_ALG:
        for num in range(NUM_TEST):
            file_path = 'Output/schd_' + str(sch_) + '_' +  str(num+1) + '.txt'
            processes = read_process_info(file_path)
            draw_gantt_chart(processes, sch_, num)