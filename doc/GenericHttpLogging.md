![Generic HTTP](image/remote.log.generichttp.jpg?raw=true)

The `format` field in log setup page is like the format in `printf` but uses the following specifiers:

| Specifier   | output  |
| -------------- |:-------------|
| %b         | Beer temperature   |
| %B         | Beer setting   |
| %f         | fridge temperature   |
| %F         | fridge setting   |
| %r         | room temperature   |
| %g         | gravity   |
| %p         | Plato   |
| %t         | tilt value from iSpindel   |
| %a         | auxiliary temperature   |
| %v         | external device voltage   |
| %u         | UNIX timestamp of last gravity update   |
| %U         | 'C' for Celsius or 'F' for Fahrenheit  |
| %m         | Mode in integer: 0:Off ,1: FridgeConst, 2:BeerConst, 3:BeerProfile  |
| %M         | Mode in character: o, b, f, p |
| %P         | Pressure reading in PSI  |

For example, let beer setting be `20.0` and beer temperature be `18.3`, if the `format` is `api_key=TheRealApiKeyHere&field1=%B&field2=%b`,
the data will be `api_key=TheRealApiKeyHere&field1=20.0&field2=18.3`.

If the method is `GET`, the data will append to the url with additional `?`, so the result will be
`http://api.thingspeak.com/update?api_key=TheRealApiKeyHere&field1=20.0&field2=18.3`.
(GET is usually not recommended.)
The `Data Type` field is used as "Content-Type" in HTTP protocol. When it is left blank, the default value of `application/x-www-form-urlencoded` will be used. The default type is good for content like `A=V1&B=V2`.

Due to the memory limitation, **HTTPS is not supported.** Therefore, if you want to send data to a service that supports only HTTPS, an additional proxy is needed.

You can use Generic HTTP format for thingspeak.com and Brewfather to get exact the same result, but you should use ubidots.com specific settings to setup ubidots.com. It is because that ubidots.com will reject all values if one of them are "null"


***
[Index](index.md).