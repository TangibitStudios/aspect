<?php
//===========================================================
//	Pomodoro processing
//
//	This script processes a POST from the Aspect Pomodoro
//	application, relaying the information into an email
//	to be used by IFFT to log Pomodoros into a Google Drive
//	spreadsheet.
//
//	POST variables:
//		ID 		: Aspect unit sending POST [string]
//		Token 	: Authorizing token [string]
//		Status 	: DONE= Pomodoro completed, INTERUPT= interrupted [string]
//		Length	: Length of pomodoro in minutes	 
//
//	Tangibit Studios 2014
//===========================================================
//	Copyright (c) 2014 Tangibit Studios LLC.  All rights reserved.
//
//	This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
//	License as published by the Free Software Foundation, either
//	version 3 of the License, or (at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//	Lesser General Public License for more details.
//
//	You should have received a copy of the GNU Lesser General Public
//	License along with this program; if not, see <http://www.gnu.org/licenses/>.
//
//===========================================================
// Token to match (weak security)
$authToken = "1111";								// CHANGE TO YOUR OWN TOKEN

// Read POST variables
$ID 	= $_POST['ID'];
$Token 	= $_POST['Token'];
$Status = $_POST['Status'];
$LengthMinutes = $_POST['Length'];

if($_POST['Token'] == $authToken)
{
	echo "ID: ". $_POST['ID']. "<br />";
	echo "Status: ". $_POST['Status']. "<br />";
	echo "Length: ". $_POST['Length']. "<br />";

	// Email
	$To      = 'LINKED_TO_IFTTT@gmail.com';			// CHANGE TO EMAIL LINKED TO IFTTT
	$Subject = '#ifttt #pomodoro';
	$Message = $ID.",".$Status.",".$LengthMinutes;
	$Headers = 'From: YOU@gmail.com' . "\r\n" .		// CHANGE TO YOUR EMAIL
    'Reply-To: YOU@gmail.com' . "\r\n" .			// CHANGE TO YOUR EMAIL
    'X-Mailer: PHP/' . phpversion();

	mail($To, $Subject, $Message, $Headers); 
	
	exit();
}
else
{
	echo "Request denied!";
	exit();
}
?>