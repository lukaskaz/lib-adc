# lib-adc
Library for using analog-to-digital converters
<br><br>
**Example of the output (w/o and w/ debug logs):**<br>
pi@raspberrypi:\~/git/lib-adc/build $ ./examples/rpi/ads1115/prog "iio:device1" 0 3.3 1<br>
First scenario -> ADCs standard read<br>
[20250320_204911.104167][INFO] Created adc ads1115 [dev/cha/max]: iio:device1/0/3.300000<br>
ADCs initiated<br>
To read press [enter]<br>
[20250320_204912.572402][DBG ] Read[]: '22778' from 'in_voltage0_raw'<br>
[20250320_204912.572594][DBG ] Read[]: '0.125000000' from 'in_voltage0_scale'<br>
[20250320_204912.572679][DBG ] Cha[0] value read: 2.850000<br>
ADCs voltage: 2.85<br>
[20250320_204912.573879][DBG ] Read[]: '22731' from 'in_voltage0_raw'<br>
[20250320_204912.574008][DBG ] Read[]: '0.125000000' from 'in_voltage0_scale'<br>
[20250320_204912.574079][DBG ] Cha[0] percent read: 86<br>
ADCs percent: 86<br>
To exit press [enter]<br>
First scenario DONE -> ADCs released<br>
[20250320_204914.715663][INFO] Removed adc ads1115 [dev/cha]: iio:device1/0<br>
Second scenario -> ADCs observed @ one shot trigger<br>
[20250320_204914.726126][DBG ] File /sys/bus/iio/devices/iio_sysfs_trigger/add_trigger elevated to: 'o+w'<br>
[20250320_204914.726437][DBG ] Written[]: '0' to '/sys/bus/iio/devices/iio_sysfs_trigger/add_trigger'<br>
[20250320_204914.736385][DBG ] File /sys/bus/iio/devices/iio_sysfs_trigger/trigger0/name elevated to: 'o+r'<br>
[20250320_204914.736575][DBG ] Read[]: 'sysfstrig0' from '/sys/bus/iio/devices/iio_sysfs_trigger/trigger0/name'<br>
[20250320_204914.745462][DBG ] File /sys/bus/iio/devices/iio_sysfs_trigger/trigger0/trigger_now elevated to: 'o+w'<br>
[20250320_204914.745615][INFO] Created oneshot trigger: /sys/bus/iio/devices/iio_sysfs_trigger/trigger0 -> sysfstrig0<br>
[20250320_204914.754422][DBG ] File /sys/bus/iio/devices/iio:device1/trigger/current_trigger elevated to: 'o+w'<br>
[20250320_204914.754683][DBG ] Written[]: 'sysfstrig0' to '/sys/bus/iio/devices/iio:device1/trigger/current_trigger'<br>
[20250320_204914.763504][DBG ] File /sys/bus/iio/devices/iio:device1/buffer/length elevated to: 'o+w'<br>
[20250320_204914.763723][DBG ] Written[]: '100' to '/sys/bus/iio/devices/iio:device1/buffer/length'<br>
[20250320_204914.772593][DBG ] File /sys/bus/iio/devices/iio:device1/scan_elements/in_voltage0_en elevated to: 'o+w'<br>
[20250320_204914.772843][DBG ] Written[]: '1' to '/sys/bus/iio/devices/iio:device1/scan_elements/in_voltage0_en'<br>
[20250320_204914.781758][DBG ] File /sys/bus/iio/devices/iio:device1/buffer/enable elevated to: 'o+w'<br>
[20250320_204914.782093][DBG ] Written[]: '1' to '/sys/bus/iio/devices/iio:device1/buffer/enable'<br>
[20250320_204914.782277][INFO] Created adc ads1115 [dev/cha/max]: iio:device1/0/3.300000<br>
[20250320_204914.782384][DBG ] Adding observer 0x55563905b120 for cha 0<br>
ADCs initiated, to trigger press [enter][20250320_204914.791262][DBG ] File /dev/iio:device1 elevated to: 'o+r'<br>
[20250320_204914.791404][INFO] Iio device monitoring started<br>
<br>
[20250320_204921.246439][DBG ] Written[]: '1' to '/sys/bus/iio/devices/iio_sysfs_trigger/trigger0/trigger_now'<br>
[20250320_204921.246682][DBG ] Trigger sysfstrig0 activated<br>
[20250320_204921.247563][DBG ] Received bytes num: 2<br>
Observer of cha: 0 got data voltage/percent: 2.84/86<br>
[20250320_204921.247699][DBG ] Cha[0] clients notified<br>
[20250320_204921.346915][DBG ] Removing observer 0x55563905b120 for cha 0<br>
[20250320_204921.347211][DBG ] Written[]: '1' to '/sys/bus/iio/devices/iio_sysfs_trigger/trigger0/trigger_now'<br>
[20250320_204921.347294][DBG ] Trigger sysfstrig0 activated<br>
To exit press [enter][20250320_204921.348292][DBG ] Received bytes num: 2<br>
[20250320_204921.348382][WARN] Cha[0] cannot notify clients<br>
<br>
Second scenario DONE -> ADCs released<br>
[20250320_204929.036239][DBG ] File /sys/bus/iio/devices/iio:device1/trigger/current_trigger elevated to: 'o+w'<br>
[20250320_204929.036614][DBG ] Written[]: '' to '/sys/bus/iio/devices/iio:device1/trigger/current_trigger'<br>
[20250320_204929.046251][DBG ] File /sys/bus/iio/devices/iio:device1/buffer/length elevated to: 'o+w'<br>
[20250320_204929.046629][DBG ] Written[]: '0' to '/sys/bus/iio/devices/iio:device1/buffer/length'<br>
[20250320_204929.056657][DBG ] File /sys/bus/iio/devices/iio:device1/scan_elements/in_voltage0_en elevated to: 'o+w'<br>
[20250320_204929.056941][DBG ] Written[]: '0' to '/sys/bus/iio/devices/iio:device1/scan_elements/in_voltage0_en'<br>
[20250320_204929.066721][DBG ] File /sys/bus/iio/devices/iio:device1/buffer/enable elevated to: 'o+w'<br>
[20250320_204929.067082][DBG ] Written[]: '0' to '/sys/bus/iio/devices/iio:device1/buffer/enable'<br>
[20250320_204929.067200][INFO] Removed adc ads1115 [dev/cha]: iio:device1/0<br>
[20250320_204929.076175][DBG ] File /sys/bus/iio/devices/iio_sysfs_trigger/remove_trigger elevated to: 'o+w'<br>
[20250320_204929.076438][DBG ] Written[]: '0' to '/sys/bus/iio/devices/iio_sysfs_trigger/remove_trigger'<br>
[20250320_204929.076509][INFO] Removed oneshot trigger: /sys/bus/iio/devices/iio_sysfs_trigger/trigger0 -> sysfstrig0<br>
Third scenario -> ADCs observed @ periodic trigger<br>
[20250320_204929.086861][DBG ] File /sys/kernel/config/iio/triggers/hrtimer/ elevated to: 'o+w'<br>
[20250320_204929.095419][DBG ] File /sys/bus/iio/devices/trigger0/name elevated to: 'o+r'<br>
[20250320_204929.095619][DBG ] Read[]: 'cnffstrig0' from '/sys/bus/iio/devices/trigger0/name'<br>
[20250320_204929.103715][DBG ] File /sys/bus/iio/devices/trigger0/sampling_frequency elevated to: 'o+w'<br>
[20250320_204929.104058][DBG ] Written[]: '0.250000' to '/sys/bus/iio/devices/trigger0/sampling_frequency'<br>
[20250320_204929.104129][INFO] Created periodic trigger: /sys/bus/iio/devices/trigger0 -> cnffstrig0<br>
[20250320_204929.112441][DBG ] File /sys/bus/iio/devices/iio:device1/trigger/current_trigger elevated to: 'o+w'<br>
[20250320_204929.112830][DBG ] Written[]: 'cnffstrig0' to '/sys/bus/iio/devices/iio:device1/trigger/current_trigger'<br>
[20250320_204929.121200][DBG ] File /sys/bus/iio/devices/iio:device1/buffer/length elevated to: 'o+w'<br>
[20250320_204929.121558][DBG ] Written[]: '100' to '/sys/bus/iio/devices/iio:device1/buffer/length'<br>
[20250320_204929.131276][DBG ] File /sys/bus/iio/devices/iio:device1/scan_elements/in_voltage0_en elevated to: 'o+w'<br>
[20250320_204929.131522][DBG ] Written[]: '1' to '/sys/bus/iio/devices/iio:device1/scan_elements/in_voltage0_en'<br>
[20250320_204929.139697][DBG ] File /sys/bus/iio/devices/iio:device1/buffer/enable elevated to: 'o+w'<br>
[20250320_204929.140038][DBG ] Written[]: '1' to '/sys/bus/iio/devices/iio:device1/buffer/enable'<br>
[20250320_204929.140250][INFO] Created adc ads1115 [dev/cha/max]: iio:device1/0/3.300000<br>
[20250320_204929.140350][DBG ] Adding observer 0x55563905b120 for cha 0<br>
ADCs initiated, trigger ongoing, to interrupt press [enter][20250320_204929.149417][DBG ] File /dev/iio:device1 elevated to: 'o+r'<br>
[20250320_204929.149704][INFO] Iio device monitoring started<br>
[20250320_204933.141146][DBG ] Received bytes num: 2<br>
Observer of cha: 0 got data voltage/percent: 2.85/86<br>
[20250320_204933.141311][DBG ] Cha[0] clients notified<br>
[20250320_204937.141145][DBG ] Received bytes num: 2<br>
Observer of cha: 0 got data voltage/percent: 2.85/86<br>
[20250320_204937.141312][DBG ] Cha[0] clients notified<br>
[20250320_204941.141140][DBG ] Received bytes num: 2<br>
Observer of cha: 0 got data voltage/percent: 2.85/86<br>
[20250320_204941.141300][DBG ] Cha[0] clients notified<br>
[20250320_204945.141497][DBG ] Received bytes num: 2<br>
Observer of cha: 0 got data voltage/percent: 3.05/92<br>
[20250320_204945.141792][DBG ] Cha[0] clients notified<br>
[20250320_204949.141446][DBG ] Received bytes num: 2<br>
Observer of cha: 0 got data voltage/percent: 3.2/97<br>
[20250320_204949.141736][DBG ] Cha[0] clients notified<br>
[20250320_204953.141407][DBG ] Received bytes num: 2<br>
Observer of cha: 0 got data voltage/percent: 2.72/82<br>
[20250320_204953.141698][DBG ] Cha[0] clients notified<br>
<br>
Third scenario DONE -> ADCs released<br>
[20250320_204955.157615][DBG ] File /sys/bus/iio/devices/iio:device1/trigger/current_trigger elevated to: 'o+w'<br>
[20250320_204955.157959][DBG ] Written[]: '' to '/sys/bus/iio/devices/iio:device1/trigger/current_trigger'<br>
[20250320_204955.167098][DBG ] File /sys/bus/iio/devices/iio:device1/buffer/length elevated to: 'o+w'<br>
[20250320_204955.167460][DBG ] Written[]: '0' to '/sys/bus/iio/devices/iio:device1/buffer/length'<br>
[20250320_204955.176714][DBG ] File /sys/bus/iio/devices/iio:device1/scan_elements/in_voltage0_en elevated to: 'o+w'<br>
[20250320_204955.177128][DBG ] Written[]: '0' to '/sys/bus/iio/devices/iio:device1/scan_elements/in_voltage0_en'<br>
[20250320_204955.186836][DBG ] File /sys/bus/iio/devices/iio:device1/buffer/enable elevated to: 'o+w'<br>
[20250320_204955.187264][DBG ] Written[]: '0' to '/sys/bus/iio/devices/iio:device1/buffer/enable'<br>
[20250320_204955.187382][INFO] Removed adc ads1115 [dev/cha]: iio:device1/0<br>
[20250320_204955.244272][INFO] Removed periodic trigger: /sys/bus/iio/devices/trigger0 -> cnffstrig0<br>
