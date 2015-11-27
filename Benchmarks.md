
<br />
<br />

# Filesystem benchmarks #

## Intro ##
We presents some filesystems comparison - ext2, ext3 and ext4 using different features. As you'll below see it can be helpful to choose proper solution.



## Metodology ##
Tests metodology:
  * kernel rev.83 with ext4 support compiled in
  * fresh beta2 installation on "My Storage" 1024MB partition
  * phone is on
  * installed only Quadrant 1.11, System Panel, Better Terminal Emulator
  * all tasks killed during the tests
  * dalvik cache cleared and second boot used for test, one minute from start
  * Quadrant runs with custom test selected: memory and I/O test only
  * 10 iterations performed but first is for warm-up, so only 9 has been taken into consideration



## Tests details ##
Ext4 partition was created using Ubuntu 10.4 and GParted with following features (by default):
```
Filesystem features: has_journal ext_attr resize_inode dir_index filetype needs_recovery extent flex_bg sparse_super large_file uninit_bg dir_nlink
```

Ext3 partition was created using Ubuntu 10.4 and GParted with following features (by default):
```
Filesystem features: has_journal ext_attr resize_inode dir_index filetype sparse_super large_file
```

<br />
**Benchmarks legend clarifications:**
  * "_ext4 ordered_" means journaling enabled with mode=ordered by command:
```
tune2fs -o journal_data_ordered /dev/XXX
```
  * "_ext4 writeback_" means journaling enabled with mode=writeback by command:
```
tune2fs -o journal_data_writeback /dev/XXX
```
  * "_ext4 writeback, no journaling_" means journaling disabled with mode=writeback by command:
```
tune2fs -o journal_data_writeback /dev/XXX
tune2fs -O ^has_journal
```
  * "_ext3 writeback_" means journaling enabled with mode=writeback, by default
  * "_ext2_" means that it's ext3 partition with journaling disabled by command:
```
 with tune2fs -O ^has_journal /dev/XXX
```


<br />
## Charts and results ##
![https://spreadsheets.google.com/oimg?key=0AqZ0_2ag0vJVdDJnbW5mSERZZ3BMdWI2cjJBMXk0X2c&oid=1&zx=8zbz97jlp7jc&nonsense=chart1.png](https://spreadsheets.google.com/oimg?key=0AqZ0_2ag0vJVdDJnbW5mSERZZ3BMdWI2cjJBMXk0X2c&oid=1&zx=8zbz97jlp7jc&nonsense=chart1.png)


**Results comment:**
As you can see, ext3 filesystem isn't good idea because of bad performance. The only pro is journaling which can be very useful while system crashes.
The best are ext2 and ext4 with journaling disabled - that was expected. Journaling is main reason for performance lost. Note, that ext4 without journaling is just ext2 with some extra features, nothing more. But your data integrity hurts, so it's not recommended option (unless you're aware what you're doind).

Recommend filesystem is ext4 with mode set to "ordered" or "writeback". The only difference is within journal mechanism. Please, read below for explanations:
http://ext4.wiki.kernel.org/index.php/Ext3_Data%3DOrdered_vs_Data%3DWriteback_mode



<br />
<br />
# Battery life benchmarks #


## SoC-to-gauge relationship ##
According to svn rev102 changes, you can see that Android battery gauge has different profile (called "Windows Mobile like") implemented. SoC state readings was used so far. Now SoC state is only used as variable to calculate value reported to system:<br />
`value = 140*(1-exp(-SoC/70))-4`
<br />
Then result is being mapped using special table, that's why it's so non-linear. For "what is SoC" explanation please refer to http://en.wikipedia.org/wiki/State_Of_Charge

![https://spreadsheets.google.com/oimg?key=0AqZ0_2ag0vJVdEZsbDVkb2tDOUFwX05BWlFxUDQzZ3c&oid=3&zx=unh9ebdirmls&nonsense=batt_chart.png](https://spreadsheets.google.com/oimg?key=0AqZ0_2ag0vJVdEZsbDVkb2tDOUFwX05BWlFxUDQzZ3c&oid=3&zx=unh9ebdirmls&nonsense=batt_chart.png)

Legend explanation:<br />
  * calculated means value is only calculated by formula
  * gague means value is calculated and mapped using special table, so this value is reported to android



## Life benchmarks ##
Soon.