<?php

// Yahoo-Weather-API-PHP class
// https://github.com/OnkelCino/Yahoo-Weather-API-PHP-class
include 'YahooWeather.class.php';

// create an instance of the class with Asbury, NJ WOEID
$weather = new YahooWeather(12761255, 'f');
$windspeed = $weather->getWindSpeed(false,0,'.');
$temperature = $weather->getTemperature();
$city = $weather->getLocationCity();
$country = $weather->getLocationCountry();
$forecast = $weather->getForecastTomorrowHigh();
//echo "It is now $temperature in $city, $country. ";
//echo "Tomorrow's temperature will reach a maximum of $forecast. ";
//$windspeed = 25;
echo "{ \"speed\":\"$windspeed\" }";
?>