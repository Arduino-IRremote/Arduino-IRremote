# Contributing
This library is the culmination of the expertise of many members of the open source community who have dedicated their time and hard work.

If you want to contribute to this project:
- Report bugs and errors
- Ask for enhancements
- Create issues and pull requests
- Tell other people about this library
- Contribute new protocols

## Guidelines
The following are some guidelines to observe when creating discussions / PRs:
#### Be friendly
It is important that we can all enjoy a safe space as we are all working on the same project and **it is okay for people to have different ideas**.
#### Use reasonable titles
Refrain from using overly long or capitalized titles as they are usually annoying and do little to encourage others to help :smile:.
#### Use the formatting style
We use the original [C Style by Kerninghan / Ritchie](https://en.wikipedia.org/wiki/Indentation_style#K&R_style) in [variant: 1TBS (OTBS)](https://en.wikipedia.org/wiki/Indentation_style#Variant:_1TBS_(OTBS)).<br/>
In short: 4 spaces indentation, no tabs, opening braces on the same line, braces are mandatory on all if/while/do, no hard line length limit.<br/>
To beautify your code, you may use the online formatter [here](https://www.freecodeformat.com/c-format.php).
#### Cover **all** occurences of the problem / addition you address with your PR
 Do not forget the documentation like it is done for existing code. Code changes without proper documentation will be rejected!

## Adding new protocols
To add a new protocol is quite straightforward. Best is too look at the existing protocols to find a similar one and modify it.<br/>
As a rule of thumb, it is easier to work with a description of the protocol rather than trying to entirely reverse-engineer the protocol.
Please include a link to the description in the header, if you found one.<br/>
The **durations** you receive are likely to be longer for marks and shorter for spaces than the protocol suggests,
but this depends on the receiver circuit in use. Most protocols use multiples of one time-unit for marks and spaces like e.g. [NEC](https://github.com/Arduino-IRremote/Arduino-IRremote/blob/master/src/ir_NEC.hpp#L62). It's easy to be off-by-one with the last bit, since the last space is not recorded by IRremote.

Try to make use of the template functions `decodePulseDistanceData()` and `sendPulseDistanceData()`.
If your protocol supports address and code fields, try to reflect this in your api like it is done in [`sendNEC(uint16_t aAddress, uint8_t aCommand, int_fast8_t aNumberOfRepeats, bool aIsRepeat)`](https://github.com/Arduino-IRremote/Arduino-IRremote/blob/master/src/ir_NEC.hpp#L96)
and [`decodeNEC()`](https://github.com/Arduino-IRremote/Arduino-IRremote/blob/master/src/ir_NEC.hpp#L194).<br/>

### Integration
To integrate your protocol, you need to extend the two functions `decode()` and `getProtocolString()` in *IRreceice.hpp*,
add macros and function declarations for sending and receiving and extend the `enum decode_type_t` in *IRremote.h*.<br/>
And at least it would be wonderful if you can provide an example how to use the new protocol.
A detailed description can be found in the [ir_Template.hpp](https://github.com/Arduino-IRremote/Arduino-IRremote/blob/master/src/ir_Template.hpp#L11) file.

### Creating API documentation
To generate the API documentation, Doxygen, as well as [Graphviz](http://www.graphviz.org/) should be installed.
(Note that on Windows, it is useful to specify the installer to add Graphviz to PATH or to do it manually.
With Doxygen and Graphviz installed, issue the command
`doxygen` from the command line in the main project directory, which will
generate the API documentation in HTML format.
The just generated `docs/index.html` can now be opened in a browser.