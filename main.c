#include <stdio.h>
#include <string.h>
#include <unistd.h>

void control_charge_speed(const char *file_path, int target_value)
{
    FILE *file = fopen(file_path, "r+");
    if (file != NULL)
    {
        int current_value;
        fscanf(file, "%d", &current_value);
        if (current_value != target_value)
        {
            rewind(file);
            fprintf(file, "%d", target_value);
        }
        fclose(file);
    }
}

int main()
{
    char buffer[128];
    char charge_type[128];
    char capacity_str[16];
    int capacity;

    while (1)
    {
        FILE *fp = fopen("/sys/class/power_supply/battery/status", "r");

        if (fp && fgets(buffer, sizeof(buffer), fp))
        {
            buffer[strcspn(buffer, "\n")] = 0;

            if (strstr(buffer, "Charging"))
            {
                FILE *charge_type_fp = fopen("/sys/class/power_supply/battery/charge_type", "r");
                FILE *capacity_fp = fopen("/sys/class/power_supply/battery/capacity", "r");

                if (charge_type_fp && fgets(charge_type, sizeof(charge_type), charge_type_fp))
                {
                    charge_type[strcspn(charge_type, "\n")] = 0;
                }

                // 标准充电逻辑
                if (strcmp(charge_type, "Standard") == 0)
                {
                    if (capacity_fp && fgets(capacity_str, sizeof(capacity_str), capacity_fp))
                    {
                        capacity_str[strcspn(capacity_str, "\n")] = 0;
                        sscanf(capacity_str, "%d", &capacity);
                    }                    

                    if (capacity <= 80)
                    {
                        // 电量 ≤ 80%，设置电流为 7,500,000 µA（7.5A），约 37W（5V x 7.5A）
                        control_charge_speed("/sys/class/power_supply/battery/constant_charge_current", 7500000);
                    }                   
                    else
                    {
                        // 电量 > 80%，设置电流为 4,000,000 µA（4A），约 20W（5V x 4A）
                        control_charge_speed("/sys/class/power_supply/battery/constant_charge_current", 4000000);
                    }
                }
                // 快速充电逻辑
                else if (strcmp(charge_type, "Fast") == 0)
                {
                    if (capacity_fp && fgets(capacity_str, sizeof(capacity_str), capacity_fp))
                    {
                        capacity_str[strcspn(capacity_str, "\n")] = 0;
                        sscanf(capacity_str, "%d", &capacity);
                    }

                    switch(capacity / 10) {
                        case 0:
                        // 电量 00% ~ 10%，设置电流为 12,400,000 µA（12.4A），约 62W（5V x 12.4A）
                        control_charge_speed("/sys/class/power_supply/battery/constant_charge_current", 12400000);
                        break;
                        case 1:
                        // 电量 10% ~ 20%，设置电流为 10,400,000 µA（7A），约 52W（5V x 10.4A）
                        control_charge_speed("/sys/class/power_supply/battery/constant_charge_current", 10400000);
                        break;
                        case 2:
                        // 电量 20% ~ 30%，设置电流为 9,000,000 µA（9A），约 45W（5V x 9A）
                        control_charge_speed("/sys/class/power_supply/battery/constant_charge_current", 9000000);
                        break;
                        case 3:
                        // 电量 30% ~ 40%，设置电流为 8,000,000 µA（8A），约 40W（5V x 8A）
                        control_charge_speed("/sys/class/power_supply/battery/constant_charge_current", 8000000);
                        break;
                        case 4:
                        case 5:
                        case 6:
                        case 7:
                        // 电量 50% ~ 80%，设置电流为 7,500,000 µA（7.5A），约 37W（5V x 7.5A）
                        control_charge_speed("/sys/class/power_supply/battery/constant_charge_current", 7500000);
                        break;
                        case 8:
                        // 电量 80% ~ 90%，设置电流为 6,000,000 µA（6A），约 30W（5V x 6A）
                        control_charge_speed("/sys/class/power_supply/battery/constant_charge_current", 6000000);
                        break;
                        case 9:
                        // 电量 90% ~ 100%，设置电流为 4,000,000 µA（5A），约 20W（5V x 4A）
                        control_charge_speed("/sys/class/power_supply/battery/constant_charge_current", 4000000);
                        break;
                    }

                    /*
                    if (capacity <= 30)
                    {
                        // 电量 ≤ 30%，设置电流为 10,000,000 µA（10A），约 50W（5V x 10A）
                        control_charge_speed("/sys/class/power_supply/battery/constant_charge_current", 10000000);
                    }
                    else if (capacity <= 80)
                    {
                        // 电量 31%~80%，线性降低电流（10000000 - (capacity - 30) * 100000）
                        // 例如：capacity = 50 → current = 8A → 40W（5V x 8A）
                        int current = 10000000 - (capacity - 30) * 100000;
                        control_charge_speed("/sys/class/power_supply/battery/constant_charge_current", current);
                    }
                    else
                    {
                        // 电量 > 80%，设置电流为 5,000,000 µA（5A），约 25W（5V x 5A）
                        control_charge_speed("/sys/class/power_supply/battery/constant_charge_current", 5000000);
                    }
                    */
                }

                if (charge_type_fp)
                    fclose(charge_type_fp);
                if (capacity_fp)
                    fclose(capacity_fp);
            }
        }

        if (fp)
            fclose(fp);

        sleep(10); // 每2秒检查一次电池状态与调节需求
    }
    return 0;
}
