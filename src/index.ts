/* eslint-disable camelcase */
import { createWriteStream, mkdirSync, unlink } from 'fs';
import * as http from 'http';
import * as childProcess from 'child_process';

const LOCAL_FILE_DIR = './records';

export default async function processCapture(payload: {
  room_id: string;
  user_id: string;
  file_url: string;
  started_at: number;
  ended_at: number;
}): Promise<void> {
  const { room_id, user_id, file_url, started_at } = payload;
  const pcapFilePath = `${LOCAL_FILE_DIR}/${room_id}/${user_id}_${started_at}.pcap`;
  const mp4FilePath = `${LOCAL_FILE_DIR}/${room_id}/${user_id}_${started_at}.mp4`;

  // Download pcap file
  await new Promise((resolve, reject) => {
    mkdirFor(pcapFilePath);
    const file = createWriteStream(pcapFilePath);
    http
      .get(file_url, (response) => {
        response.pipe(file);
        file.on('finish', () => {
          file.end(resolve);
        });
      })
      .on('error', (error) => {
        unlink(pcapFilePath, () => reject(error));
      });
  });

  // Convert
  mkdirFor(mp4FilePath);
  const args = [
    '',
    '',
    '',
    '', // These 4 arguments are unused, which are for video only
    pcapFilePath,
    '0',
    mp4FilePath,
  ];
  await new Promise<void>((resolve, reject) => {
    const convertProcess = childProcess.spawn(
      './gst-recorder/build/converter',
      args,
      {
        detached: false,
        shell: false,
      }
    );

    const timer = setTimeout(() => {
      convertProcess.kill('SIGINT');
      reject(new Error('Record convert timed out'));
    }, 6 * 3600 * 1000);

    convertProcess.stderr.setEncoding('utf-8');
    convertProcess.stdout.setEncoding('utf-8');

    convertProcess.on('message', (message) => {
      console.log(
        `[Converter] convert::process::message [pid:${convertProcess.pid}]`,
        message
      );
    });

    convertProcess.on('error', (error) => {
      console.error(error);
    });

    convertProcess.once('close', () => {
      console.log(
        `[Converter] converter::process::close [pid:${convertProcess.pid}]`
      );
      reject(new Error('Unexpected process close'));
    });

    convertProcess.stderr.on('data', (data) => {
      console.warn(`[Converter] converter::process::stderr::data`, data);
    });

    convertProcess.stdout.on('data', (data) => {
      console.log(`[Converter] converter::process::stdout::data`, data);
    });

    convertProcess.on('exit', (code) => {
      console.log(
        `[Converter] converter::process::exit [pid:${convertProcess.pid}]`
      );
      clearTimeout(timer);

      if (code === 0) {
        resolve();
      } else {
        reject(
          new Error(`Converter process exited with non-zero code: ${code}`)
        );
      }
    });
  });

  // Now you have the converted file at mp4FilePath,
  // which can be uploaded to another storage or piped to process
}

function mkdirFor(path: string): void {
  const pathFragments = path.split('/');
  pathFragments.pop();
  if (!pathFragments.length) return;
  mkdirSync(pathFragments.join('/'), { recursive: true });
}
