package com.dwyco.android;
import android.net.LocalServerSocket;
import java.io.IOException;
import java.lang.IllegalStateException;


public class SocketLock {
    public SocketLock(String name) {
        mName = name;
    }

    public final synchronized void tryLock() throws IOException {
        if (mServer == null) {
            mServer = new LocalServerSocket(mName);
        } else {
            throw new IllegalStateException("tryLock but has locked");
        }
    }

    public final synchronized boolean timedLock(int ms) {
        long expiredTime = System.currentTimeMillis() + ms;

        while (true) {
            if (System.currentTimeMillis() > expiredTime) {
                return false;
            }
            try {
                try {
                    tryLock();
                    return true;
                } catch (IOException e) {
                    // ignore the exception
                }
                Thread.sleep(10, 0);
            } catch (InterruptedException e) {
                continue;
            }
        }
    }

    public final synchronized void lock() {
        while (true) {
            try {
                try {
                    tryLock();
                    return;
                } catch (IOException e) {
                    // ignore the exception
                }
                Thread.sleep(10, 0);
            } catch (InterruptedException e) {
                continue;
            }
        }
    }

    public final synchronized void release() {
        if (mServer != null) {
            try {
                mServer.close();

            } catch (IOException e) {
                // ignore the exception
            }
        }
        mServer = null;
    }

    private final String mName;

    private LocalServerSocket mServer;
}
