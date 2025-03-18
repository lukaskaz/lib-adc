# lib-adc
Library for using analog-to-digital converters
<br><br>
**Example of the output (w/o and w/ debug logs):**<br>
pi@raspberrypi:\~/git/lib-adc/build $ ./examples/rpi/ads1115/prog "iio:device1" 0 3.3 0<br>
First scenario -> ADCs standard read<br>
[INFO] Created adc ads1115 [dev/cha/max: iio:device1/0/3.300000<br>
ADCs initiated<br>
To read press [enter]<br>
ADCs voltage: 2.991<br>
ADCs percent: 91<br>
To exit press [enter]<br>
First scenario DONE -> ADCs released<br>
[INFO] Removed adc ads1115 dev/cha: iio:device1/0<br>
Second scenario -> ADCs observed @ one shot trigger<br>
[INF] Sysfs trigger is ready to proceed<br>
[INF] Updating sysfs trigger permissions<br>
[INF] Setup successful<br>
[INFO] Created adc ads1115 [dev/cha/max: iio:device1/0/3.300000<br>
ADCs initiated, to trigger press [enter][INFO] Iio device monitoring started<br>
<br>
Cha: 0, val: 2.99162, perc: 91<br>
To exit press [enter]<br>
Second scenario DONE -> ADCs released<br>
[INFO] Removed adc ads1115 dev/cha: iio:device1/0<br>
