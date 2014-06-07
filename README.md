# pap-lab-7

Exercises to 7th and 8th lesson for subject: "Programowania aplikacji klient-serwer"

## Generowanie certyfikatu

openssl req -newkey rsa:2048 -keyout root_key.pem -out root_request.pem

openssl x509 -req -in root_request.pem -signkey root_key.pem -out root_certificate.pem

cat root_certificate.pem root_key.pem > root.pem

openssl req -newkey rsa:2048 -keyout CA_key.pem -out CA_request.pem

openssl x509 -req -in CA_request.pem -CA root.pem -CAkey root.pem -CAcreateserial -out CAcert.pem

cat CAcert.pem CA_key.pem root_certificate.pem > CA.pem


openssl genrsa 2048 > server_key.pem

openssl req -new -key server_key.pem -out server_request.pem

openssl x509 -req -in server_request.pem -CA CA.pem -CAcreateserial -CAkey CA.pem -out server_certificate.pem

cat server_certificate.pem server_key.pem CAcert.pem root_certificate.pem > server.pem