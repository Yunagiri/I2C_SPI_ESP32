#!/bin/bash
#Viet Phuong DINH
# cat << EOF
# Content−type: text/html
#
# <html><head><title>Bienvenue</title>
# <body>
# EOF
# echo "<h1>Welcome, ${REMOTE_ADDR}</h1>"

# Path to the csv file
FILE="/home/uvs/Public/data/data.csv"
if [ "$REQUEST_METHOD" == "GET" ]
 then
   CHAINE=$QUERY_STRING
   #Sending confirmation to client.
   echo -e "\nRequest received.\n"
else
   read CHAINE
fi
# Substitute all ampersands by spaces to prepare for data extraction
LISTE=${CHAINE//&/ }
# Write the header once by appending all the fields, separated by comma.
if [[ ! -f $FILE ]]
  then
    echo "No data found. Creating new file" >> output.txt
    cat > "$FILE"
    for param in $LISTE
      do
        champ=${param%=*}
        echo -n "$champ," >> "$FILE"
      done
      echo -e "\n" >> "$FILE"
else
  echo "FILE FOUND. Writing." >> output.txt
fi
for param in $LISTE
  do
    # Extracting the part before the equal sign in param
    champ=${param%=*}
    # Extracting the part after the equal sign in param
    valeur=${param#*=}
    # Substitute all plus signs in values by space
    valeur=${valeur//+/ }
    # echo "<p>champ = $champ</br>"
    # echo "valeur = ${valeur}</br></p>"
    # echo -n "$valeur, "

    # Writes the value parsed from the GET request into the data.csv file
    echo -n "$valeur, " >> $FILE
  done
  echo -e "\n" >> $FILE
# cat << EOF2
# </body></html>
# EOF2
