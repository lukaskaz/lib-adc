# lib-adc
Library for using analog-to-digital converters
<br><br>
**Example of the output (w/o and w/ debug logs):**<br>
pi@raspberrypi:\~/git/lib-adc/build $ ./examples/rpi/ads1115/prog "iio:device1" 1 3.3 10 0<br>
First scenario -> ADCs standard read<br>
[INFO] Created standard adc ads1115 [dev/cha/max]: iio:device1/1/3.300000<br>
ADCs initiated<br>
To read press [enter]<br>
ADCs voltage: 1.79<br>
ADCs percent: 54<br>
To exit press [enter]<br>
First scenario DONE -> ADCs released<br>
[INFO] Removed standard adc ads1115 [dev/cha]: iio:device1/1<br>
Second scenario -> ADCs observed @ one shot trigger<br>
[INFO] Created oneshot trigger: /sys/bus/iio/devices/iio_sysfs_trigger/trigger0 -> sysfstrig0<br>
[INFO] Created triggered adc ads1115 [dev/cha/trig/max]: iio:device1/1/sysfstrig0/3.300000<br>
ADCs initiated, to trigger press [enter][INFO] Trigger monitoring started<br>
<br>
Observer of cha: 1 got data voltage/percent: 1.779/54<br>
To exit press [enter]<br>
Second scenario DONE -> ADCs released<br>
[INFO] Removed triggered adc ads1115 [dev/cha]: iio:device1/1<br>
[INFO] Removed oneshot trigger: /sys/bus/iio/devices/iio_sysfs_trigger/trigger0 -> sysfstrig0<br>
Third scenario -> ADCs observed @ periodic trigger<br>
[INFO] Created periodic trigger: /sys/bus/iio/devices/trigger0 -> cnffstrig0@10.000000hz<br>
[INFO] Created triggered adc ads1115 [dev/cha/trig/max]: iio:device1/1/cnffstrig0/3.300000<br>
ADCs initiated, trigger ongoing, to interrupt press [enter][INFO] Trigger monitoring started<br>
Observer of cha: 1 got data voltage/percent: 1.783/54<br>
Observer of cha: 1 got data voltage/percent: 1.722/52<br>
Observer of cha: 1 got data voltage/percent: 1.685/51<br>
Observer of cha: 1 got data voltage/percent: 1.656/50<br>
Observer of cha: 1 got data voltage/percent: 1.628/49<br>
Observer of cha: 1 got data voltage/percent: 1.584/48<br>
Observer of cha: 1 got data voltage/percent: 1.543/47<br>
<br>
Third scenario DONE -> ADCs released<br>
[INFO] Removed triggered adc ads1115 [dev/cha]: iio:device1/1<br>
[INFO] Removed periodic trigger: /sys/bus/iio/devices/trigger0 -> cnffstrig0<br>
Forth scenario -> ADCs observed @ data ready events<br>
[INFO] Created data ready events driven adc ads1115 [dev/cha/fall/rise/max]: iio:device1/1/0.000000/0.000000/3.300000<br>
ADCs initiated, now... waiting for events<br>
To exit press [enter][INFO] Event monitoring started<br>
Observer of cha: 1 got data voltage/percent: 1.5548/47<br>
Observer of cha: 1 got data voltage/percent: 1.5281/46<br>
Observer of cha: 1 got data voltage/percent: 1.5016/46<br>
Observer of cha: 1 got data voltage/percent: 1.476/45<br>
Observer of cha: 1 got data voltage/percent: 1.4495/44<br>
Observer of cha: 1 got data voltage/percent: 1.424/43<br>
Observer of cha: 1 got data voltage/percent: 1.4503/44<br>
Observer of cha: 1 got data voltage/percent: 1.4753/45<br>
Observer of cha: 1 got data voltage/percent: 1.5004/45<br>
Observer of cha: 1 got data voltage/percent: 1.5264/46<br>
Observer of cha: 1 got data voltage/percent: 1.5515/47<br>
Observer of cha: 1 got data voltage/percent: 1.5774/48<br>
Observer of cha: 1 got data voltage/percent: 1.6025/49<br>
Observer of cha: 1 got data voltage/percent: 1.628/49<br>
<br>
Forth scenario DONE -> ADCs released<br>
[INFO] Removed events driven adc ads1115 [dev/cha]: iio:device1/1<br>
Fifth scenario -> ADCs observed @ limit events<br>
[INFO] Created limit events driven adc ads1115 [dev/cha/fall/rise/max]: iio:device1/1/1.750000/0.000000/3.300000<br>
ADCs initiated, now... waiting for events<br>
To exit press [enter][INFO] Event monitoring started<br>
Observer of cha: 1 got data voltage/percent: 1.7498/53<br>
Observer of cha: 1 got data voltage/percent: 1.776/54<br>
Observer of cha: 1 got data voltage/percent: 1.803/55<br>
Observer of cha: 1 got data voltage/percent: 1.8281/55<br>
Observer of cha: 1 got data voltage/percent: 1.8558/56<br>
Observer of cha: 1 got data voltage/percent: 1.8811/57<br>
Observer of cha: 1 got data voltage/percent: 1.9063/58<br>
Observer of cha: 1 got data voltage/percent: 1.9324/59<br>
Observer of cha: 1 got data voltage/percent: 1.9596/59<br>
Observer of cha: 1 got data voltage/percent: 1.934/59<br>
Observer of cha: 1 got data voltage/percent: 1.9088/58<br>
Observer of cha: 1 got data voltage/percent: 1.883/57<br>
Observer of cha: 1 got data voltage/percent: 1.8575/56<br>
Observer of cha: 1 got data voltage/percent: 1.8319/56<br>
Observer of cha: 1 got data voltage/percent: 1.8048/55<br>
Observer of cha: 1 got data voltage/percent: 1.7784/54<br>
Observer of cha: 1 got data voltage/percent: 1.7533/53<br>
<br>
Fifth scenario DONE -> ADCs released<br>
[INFO] Removed events driven adc ads1115 [dev/cha]: iio:device1/1<br>
Sixth scenario -> ADCs observed @ window events<br>
[INFO] Created window events driven adc ads1115 [dev/cha/fall/rise/max]: iio:device1/1/1.900000/2.550000/3.300000<br>
ADCs initiated, now... waiting for events<br>
To exit press [enter][INFO] Event monitoring started<br>
Observer of cha: 1 got data voltage/percent: 1.6315/49<br>
Observer of cha: 1 got data voltage/percent: 1.6591/50<br>
Observer of cha: 1 got data voltage/percent: 1.6846/51<br>
Observer of cha: 1 got data voltage/percent: 1.71/52<br>
Observer of cha: 1 got data voltage/percent: 1.7363/53<br>
Observer of cha: 1 got data voltage/percent: 1.7648/53<br>
Observer of cha: 1 got data voltage/percent: 1.7928/54<br>
Observer of cha: 1 got data voltage/percent: 1.8185/55<br>
Observer of cha: 1 got data voltage/percent: 1.844/56<br>
Observer of cha: 1 got data voltage/percent: 1.8694/57<br>
Observer of cha: 1 got data voltage/percent: 1.8951/57<br>
Observer of cha: 1 got data voltage/percent: 2.5499/77<br>
Observer of cha: 1 got data voltage/percent: 2.5761/78<br>
Observer of cha: 1 got data voltage/percent: 2.602/79<br>
Observer of cha: 1 got data voltage/percent: 2.6291/80<br>
Observer of cha: 1 got data voltage/percent: 2.6548/80<br>
Observer of cha: 1 got data voltage/percent: 2.6804/81<br>
Observer of cha: 1 got data voltage/percent: 2.706/82<br>
Observer of cha: 1 got data voltage/percent: 2.6808/81<br>
Observer of cha: 1 got data voltage/percent: 2.6551/80<br>
Observer of cha: 1 got data voltage/percent: 2.6291/80<br>
Observer of cha: 1 got data voltage/percent: 2.6036/79<br>
Observer of cha: 1 got data voltage/percent: 2.578/78<br>
Observer of cha: 1 got data voltage/percent: 2.5528/77<br>
Observer of cha: 1 got data voltage/percent: 1.8979/58<br>
Observer of cha: 1 got data voltage/percent: 1.8728/57<br>
Observer of cha: 1 got data voltage/percent: 1.8473/56<br>
Observer of cha: 1 got data voltage/percent: 1.8216/55<br>
Observer of cha: 1 got data voltage/percent: 1.7956/54<br>
Observer of cha: 1 got data voltage/percent: 1.7706/54<br>
Observer of cha: 1 got data voltage/percent: 1.7451/53<br>
<br>
Sixth scenario DONE -> ADCs released<br>
[INFO] Removed events driven adc ads1115 [dev/cha]: iio:device1/1<br>
