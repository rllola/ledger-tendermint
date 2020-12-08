import Transport from "@ledgerhq/hw-transport";
import {LedgerApp, ResponseBase} from "@zondax/ledger"

export interface ResponseAddress extends ResponseBase {
    publicKey: string;
    address: string;
}

export interface ResponseSign extends ResponseBase {
}

export interface TendermintApp extends LedgerApp {
    new(transport: Transport): TendermintApp;

    getAddressAndPubKey(path: string): Promise<ResponseAddress>;

    showAddressAndPubKey(path: string): Promise<ResponseAddress>;

    sign(path: string, message: Buffer): Promise<ResponseSign>;
}
