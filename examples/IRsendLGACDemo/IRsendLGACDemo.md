- 1) https://github.com/chaeplin/Arduino-IRremote/blob/master/examples/IRsendLGACDemo/IRsendLGACDemo.ino
- 2) Rpi python :  https://gist.github.com/chaeplin/a1220c86d88421806215
- 3) AnalysIR's IR decode : https://gist.github.com/chaeplin/a3a4b4b6b887c663bfe8
- 4) Sample raw code : https://gist.github.com/chaeplin/ab2a7ad1533c41260f0d
- 5) send raw code : https://gist.github.com/chaeplin/7c800d3166463bb51be4



=== *** ===
- (1) : fixed 
- (2) : fixed
- (3) : special(power, swing, air clean)
- (4) : change air flow, temperature
- (5) : temperature ( 15 + (5) = )
- (6) : air flow
- (7) : crc ( 3 + 4 + 5 + 6 ) & B00001111

=== *** ===

|       status   | (1)| (2)| (3)| (4)| (5)| (6)| (7)
|----------------|----|----|----|----|----|----|----
| on / 25 / mid  |1000|1000|0000|0000|1010|0010|1100
| on / 26 / mid  |1000|1000|0000|0000|1011|0010|1101      
| on / 27 / mid  |1000|1000|0000|0000|1100|0010|1110     
| on / 28 / mid  |1000|1000|0000|0000|1101|0010|1111     
| on / 25 / high |1000|1000|0000|0000|1010|0100|1110     
| on / 26 / high |1000|1000|0000|0000|1011|0100|1111     
| on / 27 / high |1000|1000|0000|0000|1100|0100|0000     
| on / 28 / high |1000|1000|0000|0000|1101|0100|0001     
| 1 up           |1000|1000|0000|1000|1101|0100|1001     
| Cool power     |1000|1000|0001|0000|0000|1100|1101     
| energy saving  |1000|1000|0001|0000|0000|0100|0101     
| power          |1000|1000|0001|0000|0000|1000|1001            
| flow/up/down   |1000|1000|0001|0011|0001|0100|1001     
| up/down off    |1000|1000|0001|0011|0001|0101|1010     
| flow/left/right|1000|1000|0001|0011|0001|0110|1011     
| left/right off |1000|1000|0001|0011|0001|0111|1100     
| Air clean      |1000|1000|1100|0000|0000|0000|1100     
| off            |1000|1000|1100|0000|0000|0101|0001 