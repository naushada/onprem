import { HttpClient, HttpHeaders, HttpParams } from '@angular/common/http';
import { Injectable } from '@angular/core';
import { Observable } from 'rxjs';
import { environment } from 'src/environments/environment';

@Injectable({
  providedIn: 'root'
})
export class HttpService {

  
  private apiURL:string = "";

  constructor(private http:HttpClient) {

    if(!environment.production) {
      this.apiURL = "http://localhost:8080"; 
    }
   }

   
   httpOptions = {
    headers: new HttpHeaders({
      'Content-Type': 'application/json'
    })
  }

  /*
  Response Format will be as below JSON Object
  {
    "status": "success/failure", ----> Mandatory
    "details": "failure reason", ----> Conditional
    "response": JSON Document    ----> Optional
  }
  */
  getlogininfo(id:string, pwd: string): Observable<string> {

    let param = "";

    if(id.length > 0 && pwd.length > 0) {
      param = `userid=${id}&password=${pwd}`;
    }

    let uri: string = "/api/v1/dms/account";
    if(this.apiURL.length > 0) {
      uri = this.apiURL + "/api/v1/dms/account";
    }

    const options = {params: new HttpParams({fromString: param})};
    return this.http.get<string>(uri, options);
  }

  /**
  Request:
   {
      "userid": "",
      "password": "",
      "roles": ["advanced", "basic", "etc"]
   }
   Response:
   {
    "status": "success/failure", ----> Mandatory
    "details": "failure reason", ----> Conditional
    "response": JSON Document    ----> Optional
  } 
   */
  createaccount(input:string): Observable<string> {
    let uri: string = "/api/v1/dms/account";
    if(this.apiURL.length > 0) {
      uri = this.apiURL + "/api/v1/dms/account";
    }

    return this.http.post<string>(uri, input, this.httpOptions);
  }

  /**
  Request:
   {
   }
   Response:
   {
    "status": "success/failure", ----> Mandatory
    "details": "failure reason", ----> Conditional
    "response": Array of JSON Document    ----> Optional
  } 
   */
  getaccounts(userid: string): Observable<string> {
    let param = "";
    param = `userid=${userid}`;

    let uri: string = "/api/v1/dms/account";
    if(this.apiURL.length > 0) {
      uri = this.apiURL + "/api/v1/dms/account";
    }

    const options = {params: new HttpParams({fromString: param})};
    return this.http.get<string>(uri, options);
  }

  /**
  Request:
   {
      "serialnumber": "",
      "productcode": "",
      "producttype": ""
   }
   Response:
   {
    "status": "success/failure", ----> Mandatory
    "details": "failure reason", ----> Conditional
    "response": JSON Document    ----> Optional
  } 
   */
  registerdevice(input:string): Observable<string> {
    let uri: string = "/api/v1/dms/register";
    if(this.apiURL.length > 0) {
      uri = this.apiURL + "/api/v1/dms/register";
    }

    return this.http.post<string>(uri, input, this.httpOptions);
  }


  /**
  Request:
   {
      "imagename": "",
      "osversion": "",
      "osbuildnumber": "",
      "producttype": ""
   }
   Response:
   {
    "status": "success/failure", ----> Mandatory
    "details": "failure reason", ----> Conditional
    "response": JSON Document    ----> Optional
  } 
   */
  uploadsoftwarerelease(input:string): Observable<string> {
    let uri: string = "/api/v1/dms/swrelease";
    if(this.apiURL.length > 0) {
      uri = this.apiURL + "/api/v1/dms/swrelease";
    }

    return this.http.post<string>(uri, input, this.httpOptions);
  }

  /*

   Response:
   {
    "status": "success/failure", ----> Mandatory
    "details": "failure reason", ----> Conditional
    "response": Array of JSON Object    ----> Optional
  } 
  */
  getsoftwarerelease(): Observable<string> {
    let uri: string = "/api/v1/dms/swrelease";
    
    if(this.apiURL.length > 0) {
      uri = this.apiURL + "/api/v1/dms/swrelease";
    }

    return this.http.get<string>(uri);
  }

  /**
   Request:
   {
    "serialnumber": "" ----- Optional
   }

   Response:
   {
    "status": "success/failure", ----> Mandatory
    "details": "failure reason", ----> Conditional
    "response": Array of JSON Object    ----> Optional
   }

   Contents of response is as below:
   {
    "serialnumber": "",
    "status": ""
    "model": "",
    "lastcommunicationdate": "",
    "osversion": "",
    "osbuildnumber":"",
    "firmware": "",
    "cellular": [{
      "carrier": "",
      "technology": "",
      "apn": "",
      "signalstrength": "",
      "ipaddress": "",
      "imei":""
    }]
   } 
   */
  getdevicedetails(serialnumber?: string) {
    let param = "all";

    if(serialnumber && serialnumber.length > 0) {
      param = `serialnumber=${serialnumber}`;
    }

    let uri: string = "/api/v1/dms/device";
    if(this.apiURL.length > 0) {
      uri = this.apiURL + "/api/v1/dms/device";
    }

    const options = {params: new HttpParams({fromString: param})};
    return this.http.get<Array<string>>(uri, options);
  }
  
}
