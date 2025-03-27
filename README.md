# lib-adc
Library for using analog-to-digital converters
<br><br>
**Example of the output (w/o and w/ debug logs):**<br>
pi@raspberrypi:\~/git/lib-adc/build $ ./examples/rpi/ads1115/prog "iio:device1" 1 3.3 10 0<br>
First scenario -> ADCs standard read<br>
[INFO] Created standard adc ads1115 [dev/cha/max]: iio:device1/1/3.300000<br>
ADCs initiated<br>
To read press [enter]<br>
ADCs direct read voltage/percent: 1.58462/48<br>
To exit press [enter]<br>
First scenario DONE -> ADCs released<br>
[INFO] Removed standard adc ads1115 [dev/cha]: iio:device1/1<br>
Second scenario -> ADCs observed @ one shot trigger<br>
[INFO] Created oneshot trigger: /sys/bus/iio/devices/iio_sysfs_trigger/trigger0 -> sysfstrig0<br>
[INFO] Created triggered adc ads1115 [dev/cha/trig/max]: iio:device1/1/sysfstrig0/3.300000<br>
ADCs initiated, to trigger press [enter][INFO] Trigger monitoring started<br>
<br>
Observer of cha: 1 got data voltage/percent: 1.58312/48<br>
To exit press [enter]<br>
Second scenario DONE -> ADCs released<br>
[INFO] Removed triggered adc ads1115 [dev/cha]: iio:device1/1<br>
[INFO] Removed oneshot trigger: /sys/bus/iio/devices/iio_sysfs_trigger/trigger0 -> sysfstrig0<br>
Third scenario -> ADCs observed @ periodic trigger<br>
[INFO] Created periodic trigger: /sys/bus/iio/devices/trigger0 -> cnffstrig0@10.000000hz<br>
[INFO] Created triggered adc ads1115 [dev/cha/trig/max]: iio:device1/1/cnffstrig0/3.300000<br>
ADCs initiated, trigger ongoing, to interrupt press [enter][INFO] Trigger monitoring started<br>
Observer of cha: 1 got data voltage/percent: 1.588/48<br>
Observer of cha: 1 got data voltage/percent: 1.549/47<br>
Observer of cha: 1 got data voltage/percent: 1.524/46<br>
Observer of cha: 1 got data voltage/percent: 1.47/45<br>
Observer of cha: 1 got data voltage/percent: 1.403/43<br>
Observer of cha: 1 got data voltage/percent: 1.372/42<br>
Observer of cha: 1 got data voltage/percent: 1.339/41<br>
Observer of cha: 1 got data voltage/percent: 1.314/40<br>
Observer of cha: 1 got data voltage/percent: 1.257/38<br>
Observer of cha: 1 got data voltage/percent: 1.211/37<br>
Observer of cha: 1 got data voltage/percent: 1.167/35<br>
Observer of cha: 1 got data voltage/percent: 1.135/34<br>
<br>
Third scenario DONE -> ADCs released<br>
[INFO] Removed triggered adc ads1115 [dev/cha]: iio:device1/1<br>
[INFO] Removed periodic trigger: /sys/bus/iio/devices/trigger0 -> cnffstrig0<br>
Forth scenario -> ADCs observed @ data ready events<br>
[INFO] Created data ready events driven adc ads1115 [dev/cha/fall/rise/max]: iio:device1/1/0.000000/0.000000/3.300000<br>
ADCs initiated, now... waiting for events<br>
To exit press [enter][INFO] Event monitoring started<br>
Observer of cha: 1 got data voltage/percent: 1.145/35<br>
Observer of cha: 1 got data voltage/percent: 1.171/35<br>
Observer of cha: 1 got data voltage/percent: 1.196/36<br>
Observer of cha: 1 got data voltage/percent: 1.222/37<br>
Observer of cha: 1 got data voltage/percent: 1.247/38<br>
Observer of cha: 1 got data voltage/percent: 1.273/39<br>
Observer of cha: 1 got data voltage/percent: 1.299/39<br>
Observer of cha: 1 got data voltage/percent: 1.325/40<br>
<br>
Forth scenario DONE -> ADCs released<br>
[INFO] Removed events driven adc ads1115 [dev/cha]: iio:device1/1<br>
Fifth scenario -> ADCs observed @ limit events<br>
[INFO] Created limit events driven adc ads1115 [dev/cha/fall/rise/max]: iio:device1/1/1.750000/0.000000/3.300000<br>
ADCs initiated, now... waiting for events<br>
To exit press [enter][INFO] Event monitoring started<br>
Observer of cha: 1 got data voltage/percent: 1.75/53<br>
Observer of cha: 1 got data voltage/percent: 1.775/54<br>
Observer of cha: 1 got data voltage/percent: 1.801/55<br>
Observer of cha: 1 got data voltage/percent: 1.829/55<br>
Observer of cha: 1 got data voltage/percent: 1.854/56<br>
Observer of cha: 1 got data voltage/percent: 1.88/57<br>
Observer of cha: 1 got data voltage/percent: 1.906/58<br>
Observer of cha: 1 got data voltage/percent: 1.933/59<br>
Observer of cha: 1 got data voltage/percent: 1.96/59<br>
Observer of cha: 1 got data voltage/percent: 1.934/59<br>
Observer of cha: 1 got data voltage/percent: 1.909/58<br>
Observer of cha: 1 got data voltage/percent: 1.884/57<br>
Observer of cha: 1 got data voltage/percent: 1.859/56<br>
Observer of cha: 1 got data voltage/percent: 1.834/56<br>
Observer of cha: 1 got data voltage/percent: 1.808/55<br>
Observer of cha: 1 got data voltage/percent: 1.781/54<br>
Observer of cha: 1 got data voltage/percent: 1.754/53<br>
<br>
Fifth scenario DONE -> ADCs released<br>
[INFO] Removed events driven adc ads1115 [dev/cha]: iio:device1/1<br>
Sixth scenario -> ADCs observed @ window events<br>
[INFO] Created window events driven adc ads1115 [dev/cha/fall/rise/max]: iio:device1/1/1.900000/2.550000/3.300000<br>
ADCs initiated, now... waiting for events<br>
To exit press [enter][INFO] Event monitoring started<br>
Observer of cha: 1 got data voltage/percent: 1.648/50<br>
Observer of cha: 1 got data voltage/percent: 1.675/51<br>
Observer of cha: 1 got data voltage/percent: 1.701/52<br>
Observer of cha: 1 got data voltage/percent: 1.729/52<br>
Observer of cha: 1 got data voltage/percent: 1.754/53<br>
Observer of cha: 1 got data voltage/percent: 1.78/54<br>
Observer of cha: 1 got data voltage/percent: 1.806/55<br>
Observer of cha: 1 got data voltage/percent: 1.832/56<br>
Observer of cha: 1 got data voltage/percent: 1.859/56<br>
Observer of cha: 1 got data voltage/percent: 1.885/57<br>
Observer of cha: 1 got data voltage/percent: 1.912/58<br>
Observer of cha: 1 got data voltage/percent: 2.549/77<br>
Observer of cha: 1 got data voltage/percent: 2.576/78<br>
Observer of cha: 1 got data voltage/percent: 2.603/79<br>
Observer of cha: 1 got data voltage/percent: 2.63/80<br>
Observer of cha: 1 got data voltage/percent: 2.656/80<br>
Observer of cha: 1 got data voltage/percent: 2.681/81<br>
Observer of cha: 1 got data voltage/percent: 2.707/82<br>
Observer of cha: 1 got data voltage/percent: 2.734/83<br>
Observer of cha: 1 got data voltage/percent: 2.761/84<br>
Observer of cha: 1 got data voltage/percent: 2.786/84<br>
Observer of cha: 1 got data voltage/percent: 2.814/85<br>
Observer of cha: 1 got data voltage/percent: 2.841/86<br>
Observer of cha: 1 got data voltage/percent: 2.866/87<br>
Observer of cha: 1 got data voltage/percent: 2.893/88<br>
Observer of cha: 1 got data voltage/percent: 2.92/88<br>
Observer of cha: 1 got data voltage/percent: 2.948/89<br>
Observer of cha: 1 got data voltage/percent: 2.974/90<br>
Observer of cha: 1 got data voltage/percent: 3/91<br>
Observer of cha: 1 got data voltage/percent: 3.025/92<br>
Observer of cha: 1 got data voltage/percent: 3.051/92<br>
Observer of cha: 1 got data voltage/percent: 3.075/93<br>
Observer of cha: 1 got data voltage/percent: 3.046/92<br>
Observer of cha: 1 got data voltage/percent: 3.02/92<br>
Observer of cha: 1 got data voltage/percent: 2.993/91<br>
Observer of cha: 1 got data voltage/percent: 2.964/90<br>
Observer of cha: 1 got data voltage/percent: 2.937/89<br>
Observer of cha: 1 got data voltage/percent: 2.912/88<br>
Observer of cha: 1 got data voltage/percent: 2.883/87<br>
Observer of cha: 1 got data voltage/percent: 2.858/87<br>
Observer of cha: 1 got data voltage/percent: 2.832/86<br>
Observer of cha: 1 got data voltage/percent: 2.804/85<br>
Observer of cha: 1 got data voltage/percent: 2.776/84<br>
Observer of cha: 1 got data voltage/percent: 2.75/83<br>
Observer of cha: 1 got data voltage/percent: 2.723/83<br>
Observer of cha: 1 got data voltage/percent: 2.695/82<br>
Observer of cha: 1 got data voltage/percent: 2.668/81<br>
Observer of cha: 1 got data voltage/percent: 2.638/80<br>
Observer of cha: 1 got data voltage/percent: 2.613/79<br>
Observer of cha: 1 got data voltage/percent: 2.587/78<br>
Observer of cha: 1 got data voltage/percent: 2.562/78<br>
Observer of cha: 1 got data voltage/percent: 2.534/77<br>
Observer of cha: 1 got data voltage/percent: 1.898/58<br>
Observer of cha: 1 got data voltage/percent: 1.873/57<br>
Observer of cha: 1 got data voltage/percent: 1.847/56<br>
Observer of cha: 1 got data voltage/percent: 1.821/55<br>
<br>
Sixth scenario DONE -> ADCs released<br>
[INFO] Removed events driven adc ads1115 [dev/cha]: iio:device1/1<br>
