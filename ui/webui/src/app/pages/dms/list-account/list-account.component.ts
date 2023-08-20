import { Component } from '@angular/core';
import { HttpService } from 'src/common/http.service';

@Component({
  selector: 'app-list-account',
  templateUrl: './list-account.component.html',
  styleUrls: ['./list-account.component.scss']
})
export class ListAccountComponent {

  selected:any;
  accounts: any[] = [];
  constructor(private http: HttpService) {
    http.getaccounts("all").subscribe((response: string) => {
      this.accounts.length = 0;
      let result = JSON.parse(JSON.stringify(response));
      if(result["status"] == "success") {
        let payload = JSON.parse(result["response"]);
        this.accounts = payload;
      }
      
    },
    (error) => {},
    () => {

    });
  }
}
