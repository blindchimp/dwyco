package main

import (
	"bytes"
	"crypto/ecdsa"
	"crypto/rand"
	"crypto/sha256"
	"crypto/x509"
	"encoding/base64"
	"encoding/json"
	"encoding/pem"
	"flag"
	"fmt"
	"io"
	"net/http"
	"os"
	"time"
)

type apsAlert struct {
	Title string `json:"title,omitempty"`
	Body  string `json:"body,omitempty"`
}

type apsPayload struct {
	Alert            apsAlert `json:"alert"`
	Badge            int      `json:"badge,omitempty"`
	Sound            string   `json:"sound,omitempty"`
	ContentAvailable int      `json:"content-available,omitempty"`
}

type pushPayload struct {
	APS       apsPayload `json:"aps"`
	Generated int64      `json:"generated"`
}

func b64urlRaw(b []byte) string {
	return base64.RawURLEncoding.EncodeToString(b)
}

func mintJWT(keyPath, keyID, teamID string) (string, error) {
	keyBytes, err := os.ReadFile(keyPath)
	if err != nil {
		return "", err
	}
	block, _ := pem.Decode(keyBytes)
	if block == nil {
		return "", fmt.Errorf("no PEM block found in %s", keyPath)
	}
	key, err := x509.ParsePKCS8PrivateKey(block.Bytes)
	if err != nil {
		return "", err
	}
	ecKey, ok := key.(*ecdsa.PrivateKey)
	if !ok {
		return "", fmt.Errorf("APNs key is not ECDSA")
	}

	headerJSON, _ := json.Marshal(map[string]string{
		"alg": "ES256",
		"kid": keyID,
	})
	claimsJSON, _ := json.Marshal(map[string]interface{}{
		"iss": teamID,
		"iat": time.Now().Unix(),
	})

	signingInput := b64urlRaw(headerJSON) + "." + b64urlRaw(claimsJSON)
	hash := sha256.Sum256([]byte(signingInput))
	r, s, err := ecdsa.Sign(rand.Reader, ecKey, hash[:])
	if err != nil {
		return "", err
	}

	// ES256 signatures are raw R+S, 32 bytes each for P-256.
	sig := make([]byte, 64)
	r.FillBytes(sig[:32])
	s.FillBytes(sig[32:])

	return signingInput + "." + b64urlRaw(sig), nil
}

func main() {
	token := flag.String("token", os.Getenv("APNS_DEVICE_TOKEN"), "APNs device token (hex)")
	keyPath := flag.String("key", os.Getenv("APNS_KEY_PATH"), "path to AuthKey_XXXX.p8")
	keyID := flag.String("key-id", os.Getenv("APNS_KEY_ID"), "APNs key id")
	teamID := flag.String("team-id", os.Getenv("APNS_TEAM_ID"), "Apple team id")
	bundleID := flag.String("bundle-id", envOrDefault("APNS_BUNDLE_ID", "com.dwyco.rando"), "app bundle id / apns topic")
	host := flag.String("host", envOrDefault("APNS_HOST", "api.push.apple.com"), "APNs host")
	title := flag.String("title", "Dwyco Rando", "notification title")
	body := flag.String("body", "New rando", "notification body")
	badge := flag.Int("badge", 1, "badge count")
	sound := flag.String("sound", "default", "notification sound")
	generated := flag.Int64("generated", time.Now().Unix(), "generated timestamp")
	flag.Parse()

	if *token == "" || *keyPath == "" || *keyID == "" || *teamID == "" {
		fmt.Fprintln(os.Stderr, "missing required APNs parameters")
		flag.Usage()
		os.Exit(2)
	}

	jwt, err := mintJWT(*keyPath, *keyID, *teamID)
	if err != nil {
		fmt.Fprintln(os.Stderr, "mint JWT:", err)
		os.Exit(1)
	}

	payload, _ := json.Marshal(pushPayload{
		APS: apsPayload{
			Alert: apsAlert{Title: *title, Body: *body},
			Badge: *badge,
			Sound: *sound,
		},
		Generated: *generated,
	})

	url := "https://" + *host + "/3/device/" + *token
	req, err := http.NewRequest(http.MethodPost, url, bytes.NewReader(payload))
	if err != nil {
		fmt.Fprintln(os.Stderr, "build request:", err)
		os.Exit(1)
	}
	req.Header.Set("authorization", "bearer "+jwt)
	req.Header.Set("apns-topic", *bundleID)
	req.Header.Set("apns-push-type", "alert")
	req.Header.Set("apns-priority", "10")
	req.Header.Set("content-type", "application/json")

	client := &http.Client{Timeout: 30 * time.Second}
	resp, err := client.Do(req)
	if err != nil {
		fmt.Fprintln(os.Stderr, "send push:", err)
		os.Exit(1)
	}
	defer resp.Body.Close()
	respBody, _ := io.ReadAll(resp.Body)
	fmt.Printf("status=%d\n%s\n", resp.StatusCode, string(respBody))
	if resp.StatusCode < 200 || resp.StatusCode >= 300 {
		os.Exit(1)
	}
}

func envOrDefault(name, fallback string) string {
	if v := os.Getenv(name); v != "" {
		return v
	}
	return fallback
}
