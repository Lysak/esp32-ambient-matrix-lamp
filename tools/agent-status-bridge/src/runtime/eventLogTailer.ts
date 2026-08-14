import { open, stat } from "node:fs/promises";

const MARKER_BYTES = 64;

export function createEventLogTailer(path: string): () => Promise<string> {
  let offset = 0;
  let marker = Buffer.alloc(0);

  return async () => {
    try {
      const stats = await stat(path);

      if (offset > 0 && marker.length > 0) {
        const markerStart = offset - marker.length;
        const handle = await open(path, "r");
        try {
          const currentMarker = Buffer.alloc(marker.length);
          const { bytesRead } = await handle.read(
            currentMarker,
            0,
            marker.length,
            markerStart,
          );

          if (bytesRead !== marker.length || !currentMarker.equals(marker)) {
            offset = 0;
            marker = Buffer.alloc(0);
          }
        } finally {
          await handle.close();
        }
      }

      if (stats.size < offset) {
        offset = 0;
        marker = Buffer.alloc(0);
      }

      if (stats.size === offset) {
        return "";
      }

      const bytesToRead = stats.size - offset;
      const handle = await open(path, "r");
      try {
        const buffer = Buffer.allocUnsafe(bytesToRead);
        const { bytesRead } = await handle.read(buffer, 0, bytesToRead, offset);
        const chunk = buffer.subarray(0, bytesRead);
        offset += bytesRead;
        marker = Buffer.concat([marker, chunk]).subarray(-MARKER_BYTES);

        return chunk.toString("utf8");
      } finally {
        await handle.close();
      }
    } catch (error) {
      if ((error as NodeJS.ErrnoException).code === "ENOENT") {
        offset = 0;
        marker = Buffer.alloc(0);
        return "";
      }
      throw error;
    }
  };
}
