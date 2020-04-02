Introduction
    The purpose of UMD Perf Profiler is to collect the performance data of media tasks, which can help us to know the workloads on GPU side. You can get the media tasks' timing, FPS and concurrency between GPU engines. 

Usage
• Step1: Build and install the latest internal-release version driver

• Step2: Update igfx_user_feature file
    Copy the igfx_user_feature.txt into /etc/ directory.
    You will see some keys with default values in igfx_user_feature.txt.
    -    Perf Profiler Buffer Size – Size of Perf profiler buffer, if not set will use the default value.
    -    Perf Profiler Enable  - Enable/Disable UMD Perf Profiler, 1 – Enable, 0 – Disable
    -    Perf Profiler Output File Name – The name of Perf Profiler output, if not set will use the default value.
    
• Step3: Run your test case
    When finish your test case, you will see a bin file under your working directory which is named as set by “Perf Profiler Output File Name” in igfx_user_feature. If you didn’t set this key, the default name should be “linux_perf_out.bin”

• Step4: Parser you bin file using MediaPerfParser
    You will get the performance report by running ./MediaPerfParser linux_perf_out.bin.
    Two csv files will be generated. You will see the per-frame data in raw data file. In the result.csv, you can get the kernel timing and tasks on each function/engine. Also you can get the total timing of this test case, FPS of encoding/decoding, and concurrency between engines.