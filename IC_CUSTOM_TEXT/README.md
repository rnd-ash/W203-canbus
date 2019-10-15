# Custom instrument cluster text

## Findings on how the W203's IC works
The IC will expect 1 or both of the following packet sequences to ID 0x1A4
1. Header text -> 2 8bit packets
2. Body text -> 3 8bit packets

### Displaying header text

#### Limitations
Must be 3 ASCII characters. If less, pad it out with spaces!



### Displaying body text
#### Limitations
Must be at MOST 9 characters, and no less than 5. Any more won't get displayed by default, and any less MUST be padded with spaces.

#### Can frame structure

|ID|DLC|Byte 1|Byte 2|Byte 3|Byte 4|Byte 5|byte 6|Byte 7|Byte 8|
|---|---|---|---|---|---|---|---|---|---|
|0x01A4|8|0x10|CSA|0x03|0x26|0x01|0x00|0x01|CSB|
|0x01A4|8|0x21|0x10|A0|A1|A2|A3|A4|A5|
|0x01A4|8|0x22|A6|A7|A8|A9|A10|0x20|0x00|

**CSA** ->  Check Sum A
**CSB** ->  Check Sum B
**A0-9** -> Body data bytes

##### Check sum A/B
Check Sum B is the number of ASCII Characters to display minus plus 2  
Check Sum A is calculated using 7+CSB

##### A0-10
This is character count dependent, but will follow the same structure for *n* characters, where *n* <= 9:  
A0-A*n*-1 -> ASCII values of characters to display  
A*n* -> 0x00
A*n+1* -> Check Sum C 
  
Any remaining bytes are left as 0x00, for example for 5 character strings, the last 4 bytes are left as 0x00  

###### Check Sum C
** By far the HARDEST one to work out! This was the magic token **

The IC appears to use the following table for calculating what the checksum should be:

|Num. of chars|Value 1|Value 2|Value 3|Value 4|
|---|---|---|---|---|
|9 ASCII chars|1073|817|561|305|
|8 ASCII chars|1090|834|578|322|
|7 ASCII chars|1136|880|624|368|
|6 ASCII chars|1121|865|609|353|
|5 ASCII chars|1135|879|623|367|

The IC Expects Check sum C to be calculated using the following algorithm:

1. Sum All the ASCII characters in text
2. Find the first value in the row for *n* number of ASCII characters, where it minus the Sum Calculated in Step 1 is less than 256. Call this the look up value
3. Check Sum C = Look up value - Sum of ASCII characters.

#### Example
Text -> "Testing!"  
Number of chars -> 8  

**First packet**  
CSB = 9+2 = 11 (0x0B)  
CSA = 11+7 = 12 (0x12)  

**Second packet** 
A0 = T (0x54)
A1 = e (0x65)
A2 = s (0x73)
A3 = t (0x74)
A4 = i (0x69)
A5 = n (0x6e)

**Third packet**  
A6 = g (0x67)
A7 = ! (0x21)
A8 = 00 (0x00)
A9 = 834-767 = 67 (0x43)
A10 = 00 (0x00)