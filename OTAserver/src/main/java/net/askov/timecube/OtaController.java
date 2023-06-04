package net.askov.timecube;

import io.quarkus.logging.Log;
import io.smallrye.mutiny.Uni;
import lombok.SneakyThrows;
import org.apache.commons.codec.digest.DigestUtils;
import org.apache.commons.io.IOUtils;

import javax.annotation.security.RolesAllowed;
import javax.ws.rs.GET;
import javax.ws.rs.Path;
import javax.ws.rs.Produces;
import javax.ws.rs.core.Context;
import javax.ws.rs.core.HttpHeaders;
import javax.ws.rs.core.MediaType;
import javax.ws.rs.core.Response;
import java.io.File;
import java.io.FileInputStream;
import java.io.InputStream;

@Path("/OTA")
public class OtaController {

    @Context
    HttpHeaders request;

    @GET
    @Path("/firmware/firmware.bin")
    @RolesAllowed("User")
    @Produces("application/octet-stream")
    public Uni<Response> currentFirmware() {

        /*
         *[
         * Cache-Control=no-cache
         * Connection=close
         * Host=gameon.askov.net:8443
         * User-Agent=ESP32-http-Update
         * x-ESP32-AP-MAC=0C:B8:15:F8:AD:1D
         * x-ESP32-chip-size=4194304
         * x-ESP32-free-space=1966080
         * x-ESP32-mode=sketch
         * x-ESP32-sdk-version=v4.4.4
         * x-ESP32-sketch-md5=5c3d8fa3c3807f5431a87409ec479d57
         * x-ESP32-sketch-sha256=54323B12F98F9004415D7F7D13A87299D2799248B0858AFCDB62690157CFDAD6
         * x-ESP32-sketch-size=1743360
         * x-ESP32-STA-MAC=0C:B8:15:F8:AD:1C
         * ]
         */
        System.out.println(request.getRequestHeaders());
        File firmwareToServe = new File(
                "/home/abr/PlatformIO/Projects/EspExplore/.pio/build/az-delivery-devkit-v4/firmware.bin");

        String servedMD5 = getCurrentMD5(firmwareToServe);
        String clientMD5 = request.getHeaderString("x-ESP32-sketch-md5");
        if (servedMD5.equalsIgnoreCase(clientMD5)) {
            Log.info("Request for current version, no update needed");
            return Uni.createFrom().item(Response.notModified().build());
        }

        Log.info("Request for version != current, so updating");
        return Uni.createFrom().item(serveFirmware(firmwareToServe));
    }

    @SneakyThrows
    private String getCurrentMD5(File firmwareToServe) {
        try (InputStream in = new FileInputStream(firmwareToServe)) {
            return DigestUtils.md5Hex(in);
        }
    }

    @SneakyThrows
    private static Response serveFirmware(File firmwareToServe) {
        return Response.ok(IOUtils.toByteArray(new FileInputStream(firmwareToServe)),
                           MediaType.APPLICATION_OCTET_STREAM_TYPE)
                       .build();
    }
}
