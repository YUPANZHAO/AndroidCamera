package com.androidcamera;

import android.provider.Settings;
import android.util.Log;

import com.autoforce.mcc4s.proto.IPCReply;
import com.autoforce.mcc4s.proto.IPCRequest;
import com.autoforce.mcc4s.proto.IPCSrvGrpc;

import org.json.JSONException;
import org.json.JSONObject;

import java.security.Key;
import java.util.Iterator;
import java.util.stream.Collector;

import io.grpc.ManagedChannel;
import io.grpc.ManagedChannelBuilder;

public class GrpcService {

    private IPCSrvGrpc.IPCSrvBlockingStub stub;

    private ManagedChannel channel;

    public GrpcService() {
        channel =
                ManagedChannelBuilder.forAddress(GlobalInfo.host, GlobalInfo.port)
                .usePlaintext().build();
        stub = IPCSrvGrpc.newBlockingStub(channel);
    }

    public String call(String data) {
        IPCRequest req = IPCRequest.newBuilder()
                .setBody(data)
                .build();
        IPCReply reply = stub.call(req);
        return reply.getBody();
    }

    public void shutdown() {
        channel.shutdown();
    }

    private JSONObject handle_reply(JSONObject reply) throws JSONException {
        switch (reply.getInt("code")) {
            case 200: return reply.getJSONObject("reply");
            case 400:
                Log.i("grpc", "msg:" + reply.getString("msg"));
                return null;
            case 500:
                Log.i("grpc", "errMsg:" + reply.getString("errMsg"));
                return null;
            default:
                return null;
        }
    }

    class KeyInfo {
        public String key;
        public String rtmp_url;
    }

    public KeyInfo genKey(String device_id) {
        try {
            JSONObject object = new JSONObject();
            object.put("method", "genkey");
            object.put("device_id", device_id);

            String json = call(object.toString());

            JSONObject info = handle_reply(new JSONObject(json));
            if(info == null) return null;

            KeyInfo ret = new KeyInfo();
            ret.key = info.getString("key");
            ret.rtmp_url = info.getString("url");
            return ret;
        } catch (JSONException e) {
            e.printStackTrace();
        }
        return null;
    }

    public interface MessageCallback {
        public void handle(String json);
    }

    public void streamCall(String id, MessageCallback cb) {
        JSONObject object = new JSONObject();
        try {
            object.put("method", "message_callback");
            object.put("id", id);
        } catch (JSONException e) {
            e.printStackTrace();
        }
        IPCRequest req = IPCRequest.newBuilder()
                .setBody(object.toString())
                .build();
        Iterator<IPCReply> iter = stub.streamCall(req);
        while(iter.hasNext()) {
            IPCReply reply = iter.next();
            Log.i("StreamMsg", reply.getBody());
            try {
                JSONObject info = handle_reply(new JSONObject(reply.getBody()));
                if(info != null) cb.handle(info.toString());
            } catch (JSONException e) {
                e.printStackTrace();
            }
        }
        Log.i("StreamMsg", "Message CallBack End");
    }

    public void deviceQuit() {
        try {
            JSONObject object = new JSONObject();
            object.put("method", "device_quit");
            object.put("id", GlobalInfo.IdentCode);

            call(object.toString());
        } catch (JSONException e) {
            e.printStackTrace();
        }
    }
}
