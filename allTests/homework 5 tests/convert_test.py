# A program to convert a formatted test input into the arguments expected by Logisim
# (Used to generate the `settings.json` file, but provided in case you for some reason
# want to test with the Logisim CLI instead of using Logisim)
import csv, sys

def convert_line(signal_dict):
    if int(signal_dict["cycle"]) < 2:
        print("WARNING: Logisim CLI behavior inconsistent for cycles < 2")

    result = [signal_dict["cycle"]]
    for signal,val in signal_dict.items():
        if signal == "cycle":
            continue

        # Make sure to convert binary strings to decimal
        result.append(f"{signal}={int(val, 2)}")

    return ",".join(result)

if len(sys.argv) != 2:
    print("Please pass a `formatted_input` CSV file for conversion")

args = []
with open(sys.argv[1]) as f:
    inputs = csv.DictReader(f)

    signal_change_list = []
    max_cycle = 0
    for row in inputs:
        signal_change_list.append(convert_line(row))
        max_cycle = max(max_cycle, int(row["cycle"]))

    args.append("-ic")
    args.append(":".join(signal_change_list))
    args.append("-c")
    args.append(f"{max_cycle}")

print("Argument list (for `settings.json`)")
print(f"[{', '.join('"' + a + '"' for a in args)}]")
print("Logisim call (java version/PATH may be incorrect, check `hwtest.py -v ALL`")
print(f"java -jar logisim_ev_cli.jar {' '.join(args)} -f <FILENAME>")
