## How to generate Figure 5
#### Example for HSS
```bash
python3 drawhssflops.py --m [path/to/hssflops.csv] --g [path/to/gohssflops.csv] --s [path/to/stflops.csv]
```
- hssnrhs.csv -> running "sbatch HSSFlops under /MatRox_RU/build/sympiler/,
- gohssnrhs.csv -> running "sbatch testGOFlops 0.0" under /MatRox_RU/libTest/ for GOFMM result.
- stnrhs..csv  -> running "sbatch testSTFlops" under /MatRox_RU/libTest/ for STRUMPACK result
#### Example for H2b
```bash
python3 drawh2bflops.py --m [path/to/h2flops.csv] --g [path/to/goh2flops.csv]
```
- h2flops.csv -> running "sbatch H2Flops" under /MatRox_RU/build/sympiler/,
- goh2flops.csv -> running "sbatch testGOFlops 0.03" under /MatRox_RU/libTest/ for GOFMM result.

use "python3 draw*flops.py -h" to check the useage
